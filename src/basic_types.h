/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
 
/**
 * @file basic_types.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_BASIC_TYPES_H_
#define  __RTCBASE_BASIC_TYPES_H_

#include <stddef.h>  // for NULL, size_t
#include <stdint.h>  // for uintptr_t and (u)int_t types.

// Use these to declare and define a static local variable that gets leaked so
// that its destructors are not called at exit.
#define RTC_DEFINE_STATIC_LOCAL(type, name, arguments) \
    static type& name = *new type arguments

#endif  //__RTCBASE_BASIC_TYPES_H_


