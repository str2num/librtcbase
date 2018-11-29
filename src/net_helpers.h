/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file net_helpers.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_NET_HELPERS_H_
#define  __RTCBASE_NET_HELPERS_H_

#include <list>

#include <netdb.h>
#include <stddef.h>

namespace rtcbase {

// rtc namespaced wrappers for inet_ntop and inet_pton so we can avoid
// the windows-native versions of these.
const char* inet_ntop(int af, const void *src, char* dst, socklen_t size);
int inet_pton(int af, const char* src, void *dst);

} // namespace rtcbase

#endif  //__RTCBASE_NET_HELPERS_H_


