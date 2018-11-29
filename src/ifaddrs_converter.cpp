/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */

/**
 * @file ifaddrs_converter.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "ifaddrs_converter.h"

namespace rtcbase {

IfAddrsConverter::IfAddrsConverter() : MemCheck("IfAddrsConverter") {}

IfAddrsConverter::~IfAddrsConverter() {}

bool IfAddrsConverter::convert_if_addrs_to_IP_address(
        const struct ifaddrs* interface,
        InterfaceAddress* ip,
        IPAddress* mask) 
{
    switch (interface->ifa_addr->sa_family) {
        case AF_INET: {
            *ip = IPAddress(
                    reinterpret_cast<sockaddr_in*>(interface->ifa_addr)->sin_addr);
            *mask = IPAddress(
                    reinterpret_cast<sockaddr_in*>(interface->ifa_netmask)->sin_addr);
            return true;
        }
        case AF_INET6: {
            int ip_attributes = IPV6_ADDRESS_FLAG_NONE;
            if (!convert_native_attributes_to_IP_attributes(interface, &ip_attributes)) {
                return false;
            }
            *ip = InterfaceAddress(
                    reinterpret_cast<sockaddr_in6*>(interface->ifa_addr)->sin6_addr,
                    ip_attributes);
            *mask = IPAddress(
                    reinterpret_cast<sockaddr_in6*>(interface->ifa_netmask)->sin6_addr);
            return true;
        }
        default: { return false; }
    }
}

bool IfAddrsConverter::convert_native_attributes_to_IP_attributes(
        const struct ifaddrs* interface,
        int* ip_attributes) 
{
    (void)interface;

    *ip_attributes = IPV6_ADDRESS_FLAG_NONE;
    return true;
}

IfAddrsConverter* create_if_addrs_converter() {
    return new IfAddrsConverter();
}

}  // namespace rtcbase


