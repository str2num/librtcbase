/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
/**
 * @file ipaddress.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <stdio.h>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "net_helpers.h"
#include "ipaddress.h"

namespace rtcbase {

// Prefixes used for categorizing IPv6 addresses.
static const in6_addr k_v4_mapped_prefix = {{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0xFF, 0xFF, 0}}};
static const in6_addr k_6to4_prefix = {{{0x20, 0x02, 0}}};
static const in6_addr k_teredo_prefix = {{{0x20, 0x01, 0x00, 0x00}}};
static const in6_addr k_v4_compatibility_prefix = {{{0}}};
static const in6_addr k_6bone_prefix = {{{0x3f, 0xfe, 0}}};

static bool is_private_v4(uint32_t ip);
static in_addr extract_mapped_address(const in6_addr& addr);

bool IPAddress::operator==(const IPAddress &other) const {
    if (_family != other._family) {
        return false;
    }
    if (_family == AF_INET) {
        return memcmp(&_u.ip4, &other._u.ip4, sizeof(_u.ip4)) == 0;
    }
    if (_family == AF_INET6) {
        return memcmp(&_u.ip6, &other._u.ip6, sizeof(_u.ip6)) == 0;
    }
    return _family == AF_UNSPEC;
}

bool IPAddress::operator!=(const IPAddress &other) const {
    return !((*this) == other);
}

bool IPAddress::operator >(const IPAddress &other) const {
    return (*this) != other && !((*this) < other);
}

bool IPAddress::operator <(const IPAddress &other) const {
    // IPv4 is 'less than' IPv6
    if (_family != other._family) {
        if (_family == AF_UNSPEC) {
            return true;
        }
        if (_family == AF_INET && other._family == AF_INET6) {
            return true;
        }
        return false;
    }
    // Comparing addresses of the same family.
    switch (_family) {
        case AF_INET: {
            return network_to_host32(_u.ip4.s_addr) <
                network_to_host32(other._u.ip4.s_addr);
        }
        case AF_INET6: {
            return memcmp(&_u.ip6.s6_addr, &other._u.ip6.s6_addr, 16) < 0;
        }
    }
    // Catches AF_UNSPEC and invalid addresses.
    return false;
}

std::ostream& operator<<(std::ostream& os, const IPAddress& ip) {
    os << ip.to_string();
    return os;
}

bool InterfaceAddress::operator==(const InterfaceAddress &other) const {
    return _ipv6_flags == other.ipv6_flags() &&
        static_cast<const IPAddress&>(*this) == other;
}

bool InterfaceAddress::operator!=(const InterfaceAddress &other) const {
    return !((*this) == other);
}

const InterfaceAddress& InterfaceAddress::operator=(
        const InterfaceAddress& other) 
{
    _ipv6_flags = other._ipv6_flags;
    static_cast<IPAddress&>(*this) = other;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const InterfaceAddress& ip) {
    os << static_cast<const IPAddress&>(ip);

    if (ip.family() == AF_INET6) {
        os << "|flags:0x" << std::hex << ip.ipv6_flags();
    }

    return os;
}

bool is_private_v4(uint32_t ip_in_host_order) {
    return ((ip_in_host_order >> 24) == 127) ||
        ((ip_in_host_order >> 24) == 10) ||
        ((ip_in_host_order >> 20) == ((172 << 4) | 1)) ||
        ((ip_in_host_order >> 16) == ((192 << 8) | 168)) ||
        ((ip_in_host_order >> 16) == ((169 << 8) | 254));
}

in_addr extract_mapped_address(const in6_addr& in6) {
    in_addr ipv4;
    ::memcpy(&ipv4.s_addr, &in6.s6_addr[12], sizeof(ipv4.s_addr));
    return ipv4;
}

in_addr IPAddress::ipv4_address() const {
    return _u.ip4;
}

in6_addr IPAddress::ipv6_address() const {
    return _u.ip6;
}

size_t IPAddress::size() const {
    switch (_family) {
        case AF_INET:
            return sizeof(in_addr);
        case AF_INET6:
            return sizeof(in6_addr);
    }
    return 0;
}

std::string IPAddress::to_string() const {
    if (_family != AF_INET && _family != AF_INET6) {
        return std::string();
    }
    char buf[INET6_ADDRSTRLEN] = {0};
    const void* src = &_u.ip4;
    if (_family == AF_INET6) {
        src = &_u.ip6;
    }
    if (!rtcbase::inet_ntop(_family, src, buf, sizeof(buf))) {
        return std::string();
    }
    return std::string(buf);
}

std::string IPAddress::to_sensitive_string() const {
#if !defined(NDEBUG)
    // Return non-stripped in debug.
    return to_string();
#else
    switch (_family) {
        case AF_INET: {
            std::string address = to_string();
            size_t find_pos = address.rfind('.');
            if (find_pos == std::string::npos) {
                return std::string();
            }
            address.resize(find_pos);
            address += ".x";
            return address;
        }
        case AF_INET6: {
            std::string result;
            result.resize(INET6_ADDRSTRLEN);
            in6_addr addr = ipv6_address();
            size_t len =
                rtcbase::sprintfn(&(result[0]), result.size(), "%x:%x:%x:x:x:x:x:x",
                        (addr.s6_addr[0] << 8) + addr.s6_addr[1],
                        (addr.s6_addr[2] << 8) + addr.s6_addr[3],
                        (addr.s6_addr[4] << 8) + addr.s6_addr[5]);
            result.resize(len);
            return result;
        }
    }
    return std::string();
#endif
}

IPAddress IPAddress::normalized() const {
    if (_family != AF_INET6) {
        return *this;
    }
    if (!IP_is_v4_mapped(*this)) {
        return *this;
    }
    in_addr addr = extract_mapped_address(_u.ip6);
    return IPAddress(addr);
}

IPAddress IPAddress::as_IPv6_address() const {
    if (_family != AF_INET) {
        return *this;
    }
    in6_addr v6addr = k_v4_mapped_prefix;
    ::memcpy(&v6addr.s6_addr[12], &_u.ip4.s_addr, sizeof(_u.ip4.s_addr));
    return IPAddress(v6addr);
}

uint32_t IPAddress::v4_address_as_host_order_integer() const {
    if (_family == AF_INET) {
        return network_to_host32(_u.ip4.s_addr);
    } else {
        return 0;
    }
}

bool IPAddress::is_nil() const {
    return IP_is_unspec(*this);
}

bool IP_from_string(const std::string& str, IPAddress* out) {
    if (!out) {
        return false;
    }
    in_addr addr;
    if (rtcbase::inet_pton(AF_INET, str.c_str(), &addr) == 0) {
        in6_addr addr6;
        if (rtcbase::inet_pton(AF_INET6, str.c_str(), &addr6) == 0) {
            *out = IPAddress();
            return false;
        }
        *out = IPAddress(addr6);
    } else {
        *out = IPAddress(addr);
    }
    return true;
}

bool IP_is_any(const IPAddress& ip) {
    switch (ip.family()) {
        case AF_INET:
            return ip == IPAddress(INADDR_ANY);
        case AF_INET6:
            return ip == IPAddress(in6addr_any) || ip == IPAddress(k_v4_mapped_prefix);
        case AF_UNSPEC:
            return false;
    }
    return false;
}

bool IP_is_loopback(const IPAddress& ip) {
    switch (ip.family()) {
        case AF_INET: {
            return ip == IPAddress(INADDR_LOOPBACK);
        }
        case AF_INET6: {
            return ip == IPAddress(in6addr_loopback);
        }
    }
    return false;
}

bool IP_is_private(const IPAddress& ip) {
    switch (ip.family()) {
        case AF_INET: {
            return is_private_v4(ip.v4_address_as_host_order_integer());
        }
        case AF_INET6: {
            return IP_is_link_local(ip) || IP_is_loopback(ip);
        }
    }
    return false;
}

bool IP_is_unspec(const IPAddress& ip) {
    return ip.family() == AF_UNSPEC;
}

bool IP_is_helper(const IPAddress& ip, const in6_addr& tomatch, int length) {
    // Helper method for checking IP prefix matches (but only on whole byte
    // lengths). Length is in bits.
    in6_addr addr = ip.ipv6_address();
    return ::memcmp(&addr, &tomatch, (length >> 3)) == 0;
}

bool IP_is_6bone(const IPAddress& ip) {
    return IP_is_helper(ip, k_6bone_prefix, 16);
}

bool IP_is_6to4(const IPAddress& ip) {
    return IP_is_helper(ip, k_6to4_prefix, 16);
}

bool IP_is_link_local(const IPAddress& ip) {
    // Can't use the helper because the prefix is 10 bits.
    in6_addr addr = ip.ipv6_address();
    return addr.s6_addr[0] == 0xFE && addr.s6_addr[1] == 0x80;
}

// According to http://www.ietf.org/rfc/rfc2373.txt, Appendix A, page 19.  An
// address which contains MAC will have its 11th and 12th bytes as FF:FE as well
// as the U/L bit as 1.
bool IP_is_mac_based(const IPAddress& ip) {
    in6_addr addr = ip.ipv6_address();
    return ((addr.s6_addr[8] & 0x02) && addr.s6_addr[11] == 0xFF &&
            addr.s6_addr[12] == 0xFE);
}

bool IP_is_site_local(const IPAddress& ip) {
    // Can't use the helper because the prefix is 10 bits.
    in6_addr addr = ip.ipv6_address();
    return addr.s6_addr[0] == 0xFE && (addr.s6_addr[1] & 0xC0) == 0xC0;
}

bool IP_is_ULA(const IPAddress& ip) {
    // Can't use the helper because the prefix is 7 bits.
    in6_addr addr = ip.ipv6_address();
    return (addr.s6_addr[0] & 0xFE) == 0xFC;
}

bool IP_is_teredo(const IPAddress& ip) {
    return IP_is_helper(ip, k_teredo_prefix, 32);
}

bool IP_is_v4_compatibility(const IPAddress& ip) {
    return IP_is_helper(ip, k_v4_compatibility_prefix, 96);
}

bool IP_is_v4_mapped(const IPAddress& ip) {
    return IP_is_helper(ip, k_v4_mapped_prefix, 96);
}

int IP_address_precedence(const IPAddress& ip) {
    // Precedence values from RFC 3484-bis. Prefers native v4 over 6to4/Teredo.
    if (ip.family() == AF_INET) {
        return 30;
    } else if (ip.family() == AF_INET6) {
        if (IP_is_loopback(ip)) {
            return 60;
        } else if (IP_is_ULA(ip)) {
            return 50;
        } else if (IP_is_v4_mapped(ip)) {
            return 30;
        } else if (IP_is_6to4(ip)) {
            return 20;
        } else if (IP_is_teredo(ip)) {
            return 10;
        } else if (IP_is_v4_compatibility(ip) || IP_is_site_local(ip) || IP_is_6bone(ip)) {
            return 1;
        } else {
            // A 'normal' IPv6 address.
            return 40;
        }
    }
    return 0;
}

IPAddress truncate_IP(const IPAddress& ip, int length) {
    if (length < 0) {
        return IPAddress();
    }
    if (ip.family() == AF_INET) {
        if (length > 31) {
            return ip;
        }
        if (length == 0) {
            return IPAddress(INADDR_ANY);
        }
        int mask = (0xFFFFFFFF << (32 - length));
        uint32_t host_order_ip = network_to_host32(ip.ipv4_address().s_addr);
        in_addr masked;
        masked.s_addr = host_to_network32(host_order_ip & mask);
        return IPAddress(masked);
    } else if (ip.family() == AF_INET6) {
        if (length > 127) {
            return ip;
        }
        if (length == 0) {
            return IPAddress(in6addr_any);
        }
        in6_addr v6addr = ip.ipv6_address();
        int position = length / 32;
        int inner_length = 32 - (length - (position * 32));
        // Note: 64bit mask constant needed to allow possible 32-bit left shift.
        uint32_t inner_mask = 0xFFFFFFFFLL << inner_length;
        uint32_t* v6_as_ints = reinterpret_cast<uint32_t*>(&v6addr.s6_addr);
        for (int i = 0; i < 4; ++i) {
            if (i == position) {
                uint32_t host_order_inner = network_to_host32(v6_as_ints[i]);
                v6_as_ints[i] = host_to_network32(host_order_inner & inner_mask);
            } else if (i > position) {
                v6_as_ints[i] = 0;
            }
        }
        return IPAddress(v6addr);
    }
    return IPAddress();
}

int count_IP_mask_bits(IPAddress mask) {
    uint32_t word_to_count = 0;
    int bits = 0;
    switch (mask.family()) {
        case AF_INET: {
            word_to_count = network_to_host32(mask.ipv4_address().s_addr);
            break;
        }
        case AF_INET6: {
            in6_addr v6addr = mask.ipv6_address();
            const uint32_t* v6_as_ints =
                reinterpret_cast<const uint32_t*>(&v6addr.s6_addr);
            int i = 0;
            for (; i < 4; ++i) {
                if (v6_as_ints[i] != 0xFFFFFFFF) {
                    break;
                }
            }
            if (i < 4) {
                word_to_count = network_to_host32(v6_as_ints[i]);
            }
            bits = (i * 32);
            break;
        }
        default: {
            return 0;
        }
    }
    if (word_to_count == 0) {
        return bits;
    }

    // Public domain bit-twiddling hack from:
    // http://graphics.stanford.edu/~seander/bithacks.html
    // Counts the trailing 0s in the word.
    unsigned int zeroes = 32;
    // This could also be written word_to_count &= -word_to_count, but
    // MSVC emits warning C4146 when negating an unsigned number.
    word_to_count &= ~word_to_count + 1;  // Isolate lowest set bit.
    if (word_to_count) { zeroes--; }
    if (word_to_count & 0x0000FFFF) { zeroes -= 16; }
    if (word_to_count & 0x00FF00FF) { zeroes -= 8; }
    if (word_to_count & 0x0F0F0F0F) { zeroes -= 4; }
    if (word_to_count & 0x33333333) { zeroes -= 2; }
    if (word_to_count & 0x55555555) { zeroes -= 1; }

    return bits + (32 - zeroes);
}

} // namespace rtcbase


