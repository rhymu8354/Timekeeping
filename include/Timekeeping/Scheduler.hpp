#pragma once

/**
 * @file Scheduler.hpp
 *
 * This module declares the Timekeeping::Scheduler class.
 *
 * © 2019 by Richard Walters
 */

#include "Clock.hpp"

#include <functional>
#include <memory>

namespace Timekeeping {

    /**
     * This extends a clock by offering the capability of calling functions
     * scheduled in advance.
     */
    class Scheduler {
        // Types
    public:
        /**
         * This is the type of function which can be scheduled to be called
         * by the scheduler.
         */
        using Callback = std::function< void() >;

        // Lifecycle Methods
    public:
        ~Scheduler() noexcept;
        Scheduler(const Scheduler&) = delete;
        Scheduler(Scheduler&&) noexcept;
        Scheduler& operator=(const Scheduler&) = delete;
        Scheduler& operator=(Scheduler&&) noexcept;

        // Public Methods
    public:
        /**
         * This is the constructor of the class.
         */
        Scheduler();

        /**
         * Set the clock object used to know when to call scheduled callbacks.
         *
         * @param[in] clock
         *     This is the object used to know when to call scheduled
         *     callbacks.
         */
        void SetClock(std::shared_ptr< Clock > clock);

        /**
         * Schedule the given callback function to be called when the
         * time returned by the clock associated with the scheduler
         * reaches the given due time.
         *
         * @param[in] callback
         *     This is the function to call later (or immediately, if the
         *     due time is now or in the past).
         *
         * @param[in] due
         *     This is the value that will be returned by the associated clock
         *     at the moment when the callback function should be called.
         *
         * @return
         *     A token is returned, which may be used to cancel the callback
         *     call before the due time.
         */
        int Schedule(
            Callback callback,
            double due
        );

        /**
         * Terminate the scheduled callback corresponding to the given token.
         *
         * @note
         *     The callback may be called anyway, if canceled close to the
         *     due time.
         *
         * @param[in] token
         *     This represents the scheduled callback to be canceled.  It was
         *     provided by the `Schedule` method when the callback was
         *     scheduled.
         */
        void Cancel(int token);

        // --------------------------------------------------------------------
        // All methods in this section are for testing only and should not
        // be used outside of the test framework.
        // ⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇⬇
        // --------------------------------------------------------------------

        /**
         * Force the scheduler to sample its clock and reevaluate scheduled
         * callbacks, even if the scheduler was asleep awaiting the next
         * scheduled time.
         */
        void WakeUp();

        // --------------------------------------------------------------------
        // ⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆⬆
        // All methods in this section are for testing only and should not
        // be used outside of the test framework.
        // --------------------------------------------------------------------

        // Private properties
    private:
        /**
         * This is the type of structure that contains the private
         * properties of the instance.  It is defined in the implementation
         * and declared here to ensure that it is scoped inside the class.
         */
        struct Impl;

        /**
         * This contains the private properties of the instance.
         */
        std::unique_ptr< Impl > impl_;
    };

}
