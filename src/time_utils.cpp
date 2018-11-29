/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file time_utils.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <sys/time.h>
#include "time_utils.h"

namespace rtcbase {

ClockInterface* g_clock = nullptr;

uint64_t system_time_nanos() {
    int64_t ticks;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ticks = k_num_nanosecs_per_sec * static_cast<int64_t>(ts.tv_sec) +
        static_cast<int64_t>(ts.tv_nsec);
    return ticks;
}

int64_t system_time_millis() {
    return static_cast<int64_t>(system_time_nanos() / k_num_nanosecs_per_millisec);
}

int64_t time_millis() {
    return static_cast<int64_t>(time_nanos() / k_num_nanosecs_per_millisec);
}

uint64_t time_micros() {
    return static_cast<uint64_t>(time_nanos() / k_num_nanosecs_per_microsec);
}

uint64_t time_nanos() {
    if (g_clock) {
        return g_clock->time_nanos();
    }
    return system_time_nanos();
}

int64_t time_diff(int64_t later, int64_t earlier) {
      return later - earlier;
}

int32_t time_diff32(uint32_t later, uint32_t earlier) {
      return later - earlier;
}

long long unix_time_micros() {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust; 
}

int64_t tm_to_seconds(const std::tm& tm) {
    static short int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    static short int cumul_mdays[12] = {0,   31,  59,  90,  120, 151,
        181, 212, 243, 273, 304, 334};
    int year = tm.tm_year + 1900;
    int month = tm.tm_mon;
    int day = tm.tm_mday - 1;  // Make 0-based like the rest.
    int hour = tm.tm_hour;
    int min = tm.tm_min;
    int sec = tm.tm_sec;

    bool expiry_in_leap_year = (year % 4 == 0 &&
            (year % 100 != 0 || year % 400 == 0));

    if (year < 1970) {
        return -1;
    }
    if (month < 0 || month > 11) {
        return -1;
    }
    if (day < 0 || day >= mdays[month] + (expiry_in_leap_year && month == 2 - 1)) {
        return -1;
    }
    if (hour < 0 || hour > 23) {
        return -1;
    }
    if (min < 0 || min > 59) {
        return -1;
    }
    if (sec < 0 || sec > 59) {
        return -1;
    }

    day += cumul_mdays[month];

    // Add number of leap days between 1970 and the expiration year, inclusive.
    day += ((year / 4 - 1970 / 4) - (year / 100 - 1970 / 100) +
            (year / 400 - 1970 / 400));

    // We will have added one day too much above if expiration is during a leap
    // year, and expiration is in January or February.
    if (expiry_in_leap_year && month <= 2 - 1) { // |month| is zero based.
        day -= 1;
    }

    // Combine all variables into seconds from 1970-01-01 00:00 (except |month|
    // which was accumulated into |day| above).
    return (((static_cast<int64_t>
                    (year - 1970) * 365 + day) * 24 + hour) * 60 + min) * 60 + sec;
}

TimestampWrapAroundHandler::TimestampWrapAroundHandler()
    : _last_ts(0), _num_wrap(-1) {}

int64_t TimestampWrapAroundHandler::unwrap(uint32_t ts) {
    if (_num_wrap == -1) {
        _last_ts = ts;
        _num_wrap = 0;
        return ts;
    }

    if (ts < _last_ts) {
        if (_last_ts >= 0xf0000000 && ts < 0x0fffffff) {
            ++_num_wrap;
        }
    } else if ((ts - _last_ts) > 0xf0000000) {
        // Backwards wrap. Unwrap with last wrap count and don't update last_ts_.
        return ts + ((_num_wrap - 1) << 32);
    }

    _last_ts = ts;
    return ts + (_num_wrap << 32);
}

} // namespace rtcbase


