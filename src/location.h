/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file location.h
 * @author str2num
 *  
 **/


#ifndef  __RTCBASE_LOCATION_H_
#define  __RTCBASE_LOCATION_H_

#include <string>

#include "stringize_macros.h"

namespace rtcbase {

// Location provides basic info where of an object was constructed, or was
// significantly brought to life.
// This is a stripped down version of:
// https://code.google.com/p/chromium/codesearch#chromium/src/base/location.h
class Location {
public:
    // Constructor should be called with a long-lived char*, such as __FILE__.
    // It assumes the provided value will persist as a global constant, and it
    // will not make a copy of it.
    //
    // TODO(deadbeef): Tracing is currently limited to 2 arguments, which is
    // why the file name and line number are combined into one argument.
    //
    // Once TracingV2 is available, separate the file name and line number.
    Location(const char* function_name, const char* file_and_line);
    Location();
    Location(const Location& other);
    Location& operator=(const Location& other);

    const char* function_name() const { return _function_name; }
    const char* file_and_line() const { return _file_and_line; }

    std::string to_string() const;

private:
    const char* _function_name;
    const char* _file_and_line;
};

// Define a macro to record the current source location.
#define RTC_FROM_HERE RTC_FROM_HERE_WITH_FUNCTION(__FUNCTION__)

#define RTC_FROM_HERE_WITH_FUNCTION(function_name) \
    ::rtcbase::Location(function_name, __FILE__ ":" STRINGIZE(__LINE__))

}  // namespace rtcbase

#endif  //__RTCBASE_LOCATION_H_


