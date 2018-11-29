/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file sigslot.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "sigslot.h"

namespace rtcbase {

pthread_mutex_t* MultiThreadedGlobal::get_mutex() {
    static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
    return &g_mutex;
}

} // namespace rtcbase


