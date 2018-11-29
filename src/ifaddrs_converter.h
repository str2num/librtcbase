/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */ 
 
/**
 * @file ifaddrs_converter.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_IFADDRS_CONVERTER_H_
#define  __RTCBASE_IFADDRS_CONVERTER_H_

#include <ifaddrs.h>

#include "memcheck.h" 
#include "ipaddress.h"

namespace rtcbase {

// This class converts native interface addresses to our internal IPAddress
// class. Subclasses should override convert_native_to_IP_attributes to implement
// the different ways of retrieving IPv6 attributes for various POSIX platforms.
class IfAddrsConverter : public MemCheck {
public:
    IfAddrsConverter();
    virtual ~IfAddrsConverter();
    virtual bool convert_if_addrs_to_IP_address(const struct ifaddrs* interface,
            InterfaceAddress* ipaddress,
            IPAddress* mask);

protected:
    virtual bool convert_native_attributes_to_IP_attributes(
            const struct ifaddrs* interface,
            int* ip_attributes);
};

IfAddrsConverter* create_if_addrs_converter();

}  // namespace rtcbase

#endif  //__RTCBASE_IFADDRS_CONVERTER_H_


