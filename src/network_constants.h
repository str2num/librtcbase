/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file network_constants.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_NETWORK_CONSTANTS_H_
#define  __RTCBASE_NETWORK_CONSTANTS_H_

namespace rtcbase {

enum AdapterType {
    ADAPTER_TYPE_UNKNOWN = 0,
    ADAPTER_TYPE_ETHERNET = 1 << 0,
    ADAPTER_TYPE_WIFI = 1 << 1,
    ADAPTER_TYPE_CELLULAR = 1 << 2,
    ADAPTER_TYPE_VPN = 1 << 3,
    ADAPTER_TYPE_LOOPBACK = 1 << 4,
    ADAPTER_TYPE_ANY = 1 << 5,
};

} // namespace rtcbase

#endif  //__RTCBASE_NETWORK_CONSTANTS_H_


