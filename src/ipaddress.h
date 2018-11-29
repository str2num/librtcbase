/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file ipaddress.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_IPADDRESS_H_
#define  __RTCBASE_IPADDRESS_H_

#include <string.h>
#include <string>
#include <vector>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "byte_order.h"

namespace rtcbase {

enum IPv6AddressFlag {
    IPV6_ADDRESS_FLAG_NONE =           0x00,

    // Temporary address is dynamic by nature and will not carry MAC
    // address.
    IPV6_ADDRESS_FLAG_TEMPORARY =      1 << 0,

    // Temporary address could become deprecated once the preferred
    // lifetime is reached. It is still valid but just shouldn't be used
    // to create new connection.
    IPV6_ADDRESS_FLAG_DEPRECATED =     1 << 1,
};

class IPAddress {
public:
    IPAddress() : _family(AF_UNSPEC) {
        ::memset(&_u, 0, sizeof(_u));
    }

    explicit IPAddress(const in_addr& ip4) : _family(AF_INET) {
        memset(&_u, 0, sizeof(_u));
        _u.ip4 = ip4;
    }

    explicit IPAddress(const in6_addr& ip6) : _family(AF_INET6) {
        _u.ip6 = ip6;
    }

    explicit IPAddress(uint32_t ip_in_host_byte_order) : _family(AF_INET) {
        memset(&_u, 0, sizeof(_u));
        _u.ip4.s_addr = host_to_network32(ip_in_host_byte_order);
    }

    IPAddress(const IPAddress& other) : _family(other._family) {
        ::memcpy(&_u, &other._u, sizeof(_u));
    }

    virtual ~IPAddress() {}
    
    const IPAddress & operator=(const IPAddress& other) {
        _family = other._family;
        ::memcpy(&_u, &other._u, sizeof(_u));
        return *this;
    }

    bool operator==(const IPAddress& other) const;
    bool operator!=(const IPAddress& other) const;
    bool operator <(const IPAddress& other) const;
    bool operator >(const IPAddress& other) const;
    friend std::ostream& operator<<(std::ostream& os, const IPAddress& addr);

    int family() const { return _family; }
    in_addr ipv4_address() const;
    in6_addr ipv6_address() const;

    // Returns the number of bytes needed to store the raw address.
    size_t size() const;

    // Wraps inet_ntop.
    std::string to_string() const;

    // Same as ToString but anonymizes it by hiding the last part.
    std::string to_sensitive_string() const;

    // Returns an unmapped address from a possibly-mapped address.
    // Returns the same address if this isn't a mapped address.
    IPAddress normalized() const;

    // Returns this address as an IPv6 address.
    // Maps v4 addresses (as ::ffff:a.b.c.d), returns v6 addresses unchanged.
    IPAddress as_IPv6_address() const;

    // For socketaddress' benefit. Returns the IP in host byte order.
    uint32_t v4_address_as_host_order_integer() const;

    // Whether this is an unspecified IP address.
    bool is_nil() const;

private:
    int _family;
    union {
        in_addr ip4;
        in6_addr ip6;
    } _u;
};

// IP class which could represent IPv6 address flags which is only
// meaningful in IPv6 case.
class InterfaceAddress : public IPAddress {
public:
    InterfaceAddress() : _ipv6_flags(IPV6_ADDRESS_FLAG_NONE) {}

    InterfaceAddress(IPAddress ip)
        : IPAddress(ip), _ipv6_flags(IPV6_ADDRESS_FLAG_NONE) {}

    InterfaceAddress(IPAddress addr, int ipv6_flags)
        : IPAddress(addr), _ipv6_flags(ipv6_flags) {}

    InterfaceAddress(const in6_addr& ip6, int ipv6_flags)
        : IPAddress(ip6), _ipv6_flags(ipv6_flags) {}

    const InterfaceAddress & operator=(const InterfaceAddress& other);

    bool operator==(const InterfaceAddress& other) const;
    bool operator!=(const InterfaceAddress& other) const;

    int ipv6_flags() const { return _ipv6_flags; }
    friend std::ostream& operator<<(std::ostream& os,
            const InterfaceAddress& addr);

private:
    int _ipv6_flags;
};

bool IP_from_string(const std::string& str, IPAddress* out);

bool IP_is_any(const IPAddress& ip);
bool IP_is_private(const IPAddress& ip);
bool IP_is_loopback(const IPAddress& ip);
bool IP_is_unspec(const IPAddress& ip);

// These are only really applicable for IPv6 addresses.
bool IP_is_6bone(const IPAddress& ip);
bool IP_is_6to4(const IPAddress& ip);
bool IP_is_link_local(const IPAddress& ip);
bool IP_is_mac_based(const IPAddress& ip);
bool IP_is_site_local(const IPAddress& ip);
bool IP_is_teredo(const IPAddress& ip);
bool IP_is_ULA(const IPAddress& ip);
bool IP_is_v4_compatibility(const IPAddress& ip);
bool IP_is_v4_mapped(const IPAddress& ip);

// Returns the precedence value for this IP as given in RFC3484.
int IP_address_precedence(const IPAddress& ip);

// Returns 'ip' truncated to be 'length' bits long.
IPAddress truncate_IP(const IPAddress& ip, int length);

// Returns the number of contiguously set bits, counting from the MSB in network
// byte order, in this IPAddress. Bits after the first 0 encountered are not
// counted.
int count_IP_mask_bits(IPAddress mask);

} // namespace rtcbase

#endif  //__RTCBASE_IPADDRESS_H_


