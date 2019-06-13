#pragma once

/**
 * @file Clock.hpp
 *
 * This module declares the Timekeeping::Clock interface.
 *
 * © 2019 by Richard Walters
 */

namespace Timekeeping {

    /**
     * This represents an object which can be used to determine what
     * the current time is during the execution of the program.
     */
    class Clock {
    public:
        // Methods

        /**
         * This method returns the current time, in seconds, since the
         * clock's reference point (typically UNIX epoch, or Midnight UTC
         * January 1, 1970).
         *
         * @return
         *     The current time in seconds is returned.
         */
        virtual double GetCurrentTime() = 0;
    };

}
