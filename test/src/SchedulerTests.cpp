/**
 * @file SchedulerTests.cpp
 *
 * This module contains the unit tests of the Timekeeping::Scheduler class.
 *
 * © 2018 by Richard Walters
 */

#include <future>
#include <gtest/gtest.h>
#include <Timekeeping/Clock.hpp>
#include <Timekeeping/Scheduler.hpp>

namespace {

    /**
     * This is a fake clock which is used to test the scheduler.
     */
    struct MockClock
        : public Timekeeping::Clock
    {
        // Properties

        double currentTime = 0.0;

        // Methods

        // Clock

        virtual double GetCurrentTime() override {
            return currentTime;
        }
    };

}

/**
 * This is the test fixture for these tests, providing common
 * setup and teardown for each test.
 */
struct SchedulerTests
    : public ::testing::Test
{
    // Properties

    /**
     * This is the unit under test.
     */
    Timekeeping::Scheduler scheduler;

    /**
     * This is the mock for the clock which drives the scheduler.
     */
    std::shared_ptr< MockClock > mockClock = std::make_shared< MockClock >();

    // Methods

    void AdvanceMockClock(double deltaTime) {
        mockClock->currentTime += deltaTime;
        scheduler.WakeUp();
    }

    // ::testing::Test

    virtual void SetUp() override {
        scheduler.SetClock(mockClock);
    }

    virtual void TearDown() override {
    }
};

TEST_F(SchedulerTests, Schedule) {
    // Arrange
    std::promise< void > calledBack;
    const auto callback = [&calledBack]{ calledBack.set_value(); };
    auto calledBackFuture = calledBack.get_future();

    // Act
    (void)scheduler.Schedule(callback, 10.0);
    AdvanceMockClock(5.0);
    const auto wasCalledEarly = (
        calledBackFuture.wait_for(std::chrono::milliseconds(100))
        == std::future_status::ready
    );
    AdvanceMockClock(5.001);
    const auto wasCalledOnTime = (
        calledBackFuture.wait_for(std::chrono::milliseconds(100))
        == std::future_status::ready
    );

    // Assert
    EXPECT_FALSE(wasCalledEarly);
    EXPECT_TRUE(wasCalledOnTime);
}

TEST_F(SchedulerTests, Cancel) {
    // Arrange
    std::promise< void > calledBack;
    const auto callback = [&calledBack]{ calledBack.set_value(); };
    auto calledBackFuture = calledBack.get_future();
    const auto token = scheduler.Schedule(callback, 10.0);

    // Act
    AdvanceMockClock(5.0);
    scheduler.Cancel(token);
    AdvanceMockClock(5.001);
    const auto wasCalledOnTime = (
        calledBackFuture.wait_for(std::chrono::seconds(0))
        == std::future_status::ready
    );

    // Assert
    EXPECT_FALSE(wasCalledOnTime);
}

TEST_F(SchedulerTests, ScheduleWithoutClock) {
    // Arrange
    scheduler = Timekeeping::Scheduler();
    std::promise< void > calledBack;
    const auto callback = [&calledBack]{ calledBack.set_value(); };
    auto calledBackFuture = calledBack.get_future();

    // Act
    const auto token = scheduler.Schedule(callback, 10.0);
    AdvanceMockClock(10.001);
    const auto wasCalledOnTime = (
        calledBackFuture.wait_for(std::chrono::seconds(0))
        == std::future_status::ready
    );

    // Assert
    EXPECT_EQ(0, token);
    EXPECT_FALSE(wasCalledOnTime);
}

TEST_F(SchedulerTests, GetClock) {
    // Arrange
    scheduler = Timekeeping::Scheduler();
    scheduler.SetClock(mockClock);

    // Act
    const auto clock = scheduler.GetClock();

    // Assert
    EXPECT_EQ(mockClock, clock);
}
