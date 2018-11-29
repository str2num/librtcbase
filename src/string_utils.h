/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file string_utils.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_STRING_UTILS_H_
#define  __RTCBASE_STRING_UTILS_H_

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include "basic_types.h"

///////////////////////////////////////////////////////////////////////////////
// Generic string/memory utilities
///////////////////////////////////////////////////////////////////////////////

#define STACK_ARRAY(TYPE, LEN) static_cast<TYPE*>(::alloca((LEN)*sizeof(TYPE)))

namespace rtcbase {

// Some compilers (clang specifically) require vsprintfn be defined before
// sprintfn.
template<class CTYPE>
size_t vsprintfn(CTYPE* buffer, size_t buflen, const CTYPE* format,
        va_list args) 
{
    int len = vsnprintf(buffer, buflen, format, args);
    if ((len < 0) || (static_cast<size_t>(len) >= buflen)) {
        len = static_cast<int>(buflen - 1);
        buffer[len] = 0;
    }
    return len;
}

template<class CTYPE>
size_t sprintfn(CTYPE* buffer, size_t buflen, const CTYPE* format, ...);
template<class CTYPE>
size_t sprintfn(CTYPE* buffer, size_t buflen, const CTYPE* format, ...) {
    va_list args;
    va_start(args, format);
    size_t len = vsprintfn(buffer, buflen, format, args);
    va_end(args);
    return len;
}

// Remove leading and trailing whitespaces.
std::string string_trim(const std::string& s);

} // rtcbase

#endif  //__RTCBASE_STRING_UTILS_H_


