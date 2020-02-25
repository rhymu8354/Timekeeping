/**
 * @file Scheduler.cpp
 *
 * This module contains the implementation of the Timekeeping::Scheduler class.
 *
 * © 2018 by Richard Walters
 */

#include <condition_variable>
#include <map>
#include <math.h>
#include <mutex>
#include <queue>
#include <thread>
#include <Timekeeping/Scheduler.hpp>

namespace {

    struct ScheduledCallback {
        // Properties

        int token;
        double due;
        Timekeeping::Scheduler::Callback callback;

        // Methods

        bool operator>(const ScheduledCallback& other) const {
            return due > other.due;
        }

    };

}

namespace Timekeeping {

    /**
     * This contains the private properties of a Scheduler class instance.
     */
    struct Scheduler::Impl {
        // Properties

        std::shared_ptr< Clock > clock;
        std::priority_queue<
            ScheduledCallback,
            std::vector< ScheduledCallback >,
            std::greater< ScheduledCallback >
        > scheduledCallbacks;
        std::thread worker;
        std::condition_variable_any wakeWorker;
        bool stopWorker = false;
        std::recursive_mutex mutex;
        int nextToken = 1;
        std::map< int, bool > isCallbackCanceled;

        // Lifecycle

        ~Impl() noexcept {
            if (!worker.joinable()) {
                return;
            }
            std::unique_lock< decltype(mutex) > lock(mutex);
            stopWorker = true;
            wakeWorker.notify_all();
            lock.unlock();
            worker.join();
        }
        Impl(const Impl&) noexcept = delete;
        Impl(Impl&&) = default;
        Impl& operator=(const Impl&) = delete;
        Impl& operator=(Impl&&) = default;

        // Methods

        Impl() {
            worker = std::thread(&Impl::Worker, this);
        }

        void Worker() {
            std::unique_lock< decltype(mutex) > lock(mutex);
            while (!stopWorker) {
                if (scheduledCallbacks.empty()) {
                    wakeWorker.wait(lock);
                } else {
                    const auto& nextInSchedule = scheduledCallbacks.top();
                    const auto now = clock->GetCurrentTime();
                    const auto waitTimeSeconds = nextInSchedule.due - now;
                    const auto isNotCanceled = !isCallbackCanceled[nextInSchedule.token];
                    if (
                        (waitTimeSeconds > 0)
                        && isNotCanceled
                    ) {
                        wakeWorker.wait_for(
                            lock,
                            std::chrono::milliseconds(
                                (int)ceil(waitTimeSeconds * 1000.0)
                            )
                        );
                    } else if (isNotCanceled) {
                        auto callback = nextInSchedule.callback;
                        (void)isCallbackCanceled.erase(nextInSchedule.token);
                        scheduledCallbacks.pop();
                        lock.unlock();
                        callback();
                        lock.lock();
                    } else {
                        (void)isCallbackCanceled.erase(nextInSchedule.token);
                        scheduledCallbacks.pop();
                    }
                }
            }
        }
    };

    Scheduler::~Scheduler() noexcept = default;
    Scheduler::Scheduler(Scheduler&&) noexcept = default;
    Scheduler& Scheduler::operator=(Scheduler&& other) noexcept = default;

    Scheduler::Scheduler()
        : impl_(new Impl())
    {
    }

    std::shared_ptr< Clock > Scheduler::GetClock() const {
        return impl_->clock;
    }

    void Scheduler::SetClock(std::shared_ptr< Clock > clock) {
        impl_->clock = clock;
    }

    int Scheduler::Schedule(
        Callback callback,
        double due
    ) {
        if (impl_->clock == nullptr) {
            return 0;
        }
        ScheduledCallback scheduledCallback;
        scheduledCallback.callback = callback;
        scheduledCallback.due = due;
        std::lock_guard< decltype(impl_->mutex) > lock(impl_->mutex);
        const auto token = impl_->nextToken++;
        impl_->isCallbackCanceled[token] = false;
        scheduledCallback.token = token;
        impl_->scheduledCallbacks.push(std::move(scheduledCallback));
        impl_->wakeWorker.notify_one();
        return token;
    }

    void Scheduler::Cancel(int token) {
        std::lock_guard< decltype(impl_->mutex) > lock(impl_->mutex);
        auto isCallbackCanceledEntry = impl_->isCallbackCanceled.find(token);
        if (isCallbackCanceledEntry != impl_->isCallbackCanceled.end()) {
            isCallbackCanceledEntry->second = true;
        }
    }

    void Scheduler::WakeUp() {
        std::lock_guard< decltype(impl_->mutex) > lock(impl_->mutex);
        impl_->wakeWorker.notify_one();
    }

}
