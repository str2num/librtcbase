/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file net_helpers.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <ifaddrs.h>

#include "byte_order.h"
#include "net_helpers.h"

namespace rtcbase {

const char* inet_ntop(int af, const void *src, char* dst, socklen_t size) {
    return ::inet_ntop(af, src, dst, size);
}

int inet_pton(int af, const char* src, void *dst) {
    return ::inet_pton(af, src, dst);
}

} // namespace rtcbase


