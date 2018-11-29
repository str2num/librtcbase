/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */ 
 
/**
 * @file platform_thread_types.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_PLATFORM_THREAD_TYPES_H_
#define  __RTCBASE_PLATFORM_THREAD_TYPES_H_

#include <pthread.h>
#include <unistd.h>

namespace rtcbase {

typedef pid_t PlatformThreadId;
typedef pthread_t PlatformThreadRef;

} // namespace rtcbase

#endif  //__RTCBASE_PLATFORM_THREAD_TYPES_H_


