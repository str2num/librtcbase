/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file time_utils.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_TIME_UTILS_H_
#define  __RTCBASE_TIME_UTILS_H_

#include <ctime>
#include <time.h>

#include "basic_types.h"

namespace rtcbase {

static const int64_t k_num_millisecs_per_sec = INT64_C(1000);
static const int64_t k_num_microsecs_per_sec = INT64_C(1000000);
static const int64_t k_num_nanosecs_per_sec = INT64_C(1000000000);

static const int64_t k_num_microsecs_per_millisec =
    k_num_microsecs_per_sec / k_num_millisecs_per_sec;
static const int64_t k_num_nanosecs_per_millisec =
    k_num_nanosecs_per_sec / k_num_millisecs_per_sec;
static const int64_t k_num_nanosecs_per_microsec =
    k_num_nanosecs_per_sec / k_num_microsecs_per_sec;

class ClockInterface {
public:
    virtual ~ClockInterface() {}
    virtual uint64_t time_nanos() const = 0;
};

// Returns the actual system time, even if a clock is set for testing.
// Useful for timeouts while using a test clock, or for logging.
uint64_t system_time_nanos();
int64_t system_time_millis();

// Number of milliseconds that would elapse between 'earlier' and 'later'
// timestamps.  The value is negative if 'later' occurs before 'earlier'.
int64_t time_diff(int64_t later, int64_t earlier);
int32_t time_diff32(uint32_t later, uint32_t earlier);

// Returns the current time in milliseconds in 64 bits.
int64_t time_millis();

// Returns the current time in microseconds.
uint64_t time_micros();

// Returns the current time in nanoseconds.
uint64_t time_nanos();

// Return the UNIX time in microseconds
long long unix_time_micros();

// Convert from std::tm, which is relative to 1900-01-01 00:00 to number of
// seconds from 1970-01-01 00:00 ("epoch").  Don't return time_t since that
// is still 32 bits on many systems.
int64_t tm_to_seconds(const std::tm& tm);

class TimestampWrapAroundHandler {
public:
    TimestampWrapAroundHandler();

    int64_t unwrap(uint32_t ts);

private:
    uint32_t _last_ts;
    int64_t _num_wrap;
};

} // namespace rtcbase

#endif  //__RTCBASE_TIME_UTILS_H_


