/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 

/**
 * @file socket_address.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "byte_order.h"
#include "logging.h"
#include "net_helpers.h"
#include "socket_address.h"

namespace rtcbase {

SocketAddress::SocketAddress() {
    clear();
}

SocketAddress::SocketAddress(const std::string& hostname, int port) {
    set_IP(hostname);
    set_port(port);
}

SocketAddress::SocketAddress(uint32_t ip_as_host_order_integer, int port) {
    set_IP(IPAddress(ip_as_host_order_integer));
    set_port(port);
}

SocketAddress::SocketAddress(const IPAddress& ip, int port) {
    set_IP(ip);
    set_port(port);
}

SocketAddress::SocketAddress(const SocketAddress& addr) {
    this->operator=(addr);
}

void SocketAddress::clear() {
    _hostname.clear();
    _literal = false;
    _ip = IPAddress();
    _port = 0;
    _scope_id = 0;
}

bool SocketAddress::is_nil() const {
    return _hostname.empty() && IP_is_unspec(_ip) && 0 == _port;
}

SocketAddress& SocketAddress::operator=(const SocketAddress& addr) {
    _hostname = addr._hostname;
    _ip = addr._ip;
    _port = addr._port;
    _literal = addr._literal;
    _scope_id = addr._scope_id;
    return *this;
}

void SocketAddress::set_IP(uint32_t ip_as_host_order_integer) {
    _hostname.clear();
    _literal = false;
    _ip = IPAddress(ip_as_host_order_integer);
    _scope_id = 0;
}

void SocketAddress::set_IP(const IPAddress& ip) {
    _hostname.clear();
    _literal = false;
    _ip = ip;
    _scope_id = 0;
}

void SocketAddress::set_IP(const std::string& hostname) {
    _hostname = hostname;
    _literal = IP_from_string(hostname, &_ip);
    if (!_literal) {
        _ip = IPAddress();
    }
    _scope_id = 0;
}

void SocketAddress::set_port(int port) {
    if (port < 0 || port >= 65536) {
        return;
    }
    _port = static_cast<uint16_t>(port);
}

const IPAddress& SocketAddress::ipaddr() const {
    return _ip;
}

uint16_t SocketAddress::port() const {
    return _port;
}

std::string SocketAddress::host_as_URI_string() const {
    // If the hostname was a literal IP string, it may need to have square
    // brackets added (for SocketAddress::ToString()).
    if (!_literal && !_hostname.empty()) {
        return _hostname;
    }
    if (_ip.family() == AF_INET6) {
        return "[" + _ip.to_string() + "]";
    } else {
        return _ip.to_string();
    }
}

std::string SocketAddress::port_as_string() const {
    std::ostringstream ost;
    ost << _port;
    return ost.str();
}

std::string SocketAddress::host_as_sensitive_URI_string() const {
    // If the hostname was a literal IP string, it may need to have square
    // brackets added (for SocketAddress::ToString()).
    if (!_literal && !_hostname.empty()) {
        return _hostname;
    }
    if (_ip.family() == AF_INET6) {
        return "[" + _ip.to_sensitive_string() + "]";
    } else {
        return _ip.to_sensitive_string();
    }
}

std::string SocketAddress::to_string() const {
    std::ostringstream ost;
    ost << *this;
    return ost.str();
}

std::string SocketAddress::to_sensitive_string() const {
    std::ostringstream ost;
    ost << host_as_sensitive_URI_string() << ":" << port();
    return ost.str();
}

bool SocketAddress::from_string(const std::string& str) {
    if (str.at(0) == '[') {
        std::string::size_type closebracket = str.rfind(']');
        if (closebracket != std::string::npos) {
            std::string::size_type colon = str.find(':', closebracket);
            if (colon != std::string::npos && colon > closebracket) {
                set_port(strtoul(str.substr(colon + 1).c_str(), NULL, 10));
                set_IP(str.substr(1, closebracket - 1));
            } else {
                return false;
            }
        }
    } else {
        std::string::size_type pos = str.find(':');
        if (std::string::npos == pos)
            return false;
        set_port(strtoul(str.substr(pos + 1).c_str(), NULL, 10));
        set_IP(str.substr(0, pos));
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const SocketAddress& addr) {
    os << addr.host_as_URI_string() << ":" << addr.port();
    return os;
}

bool SocketAddress::is_any_IP() const {
    return IP_is_any(_ip);
}

bool SocketAddress::is_private_IP() const {
    return IP_is_private(_ip);
}

bool SocketAddress::is_unresolved_IP() const {
    return IP_is_unspec(_ip) && !_literal && !_hostname.empty();
}

bool SocketAddress::operator==(const SocketAddress& addr) const {
    return equal_IPs(addr) && equal_ports(addr);
}

bool SocketAddress::operator<(const SocketAddress& addr) const {
    if (_ip != addr._ip) {
        return _ip < addr._ip;
    }

    // We only check hostnames if both IPs are ANY or unspecified.  This matches
    // EqualIPs().
    if ((IP_is_any(_ip) || IP_is_unspec(_ip)) && _hostname != addr._hostname) {
        return _hostname < addr._hostname;
    }

    return _port < addr._port;
}

bool SocketAddress::equal_IPs(const SocketAddress& addr) const {
    return (_ip == addr._ip) &&
        ((!IP_is_any(_ip) && !IP_is_unspec(_ip)) || (_hostname == addr._hostname));
}

bool SocketAddress::equal_ports(const SocketAddress& addr) const {
    return (_port == addr._port);
}

static size_t to_sockaddr_storage_helper(sockaddr_storage* addr,
        IPAddress ip,
        uint16_t port,
        int scope_id) 
{
    memset(addr, 0, sizeof(sockaddr_storage));
    addr->ss_family = static_cast<unsigned short>(ip.family());
    if (addr->ss_family == AF_INET6) {
        sockaddr_in6* saddr = reinterpret_cast<sockaddr_in6*>(addr);
        saddr->sin6_addr = ip.ipv6_address();
        saddr->sin6_port = host_to_network16(port);
        saddr->sin6_scope_id = scope_id;
        return sizeof(sockaddr_in6);
    } else if (addr->ss_family == AF_INET) {
        sockaddr_in* saddr = reinterpret_cast<sockaddr_in*>(addr);
        saddr->sin_addr = ip.ipv4_address();
        saddr->sin_port = host_to_network16(port);
        return sizeof(sockaddr_in);
    }
    return 0;
}

size_t SocketAddress::to_sockaddr_storage(sockaddr_storage* addr) const {
    return to_sockaddr_storage_helper(addr, _ip, _port, _scope_id);
}

bool socket_address_from_sockaddr_storage(const sockaddr_storage& addr,
        SocketAddress* out) 
{
    if (!out) {
        return false;
    }
    if (addr.ss_family == AF_INET) {
        const sockaddr_in* saddr = reinterpret_cast<const sockaddr_in*>(&addr);
        *out = SocketAddress(IPAddress(saddr->sin_addr),
                network_to_host16(saddr->sin_port));
        return true;
    } else if (addr.ss_family == AF_INET6) {
        const sockaddr_in6* saddr = reinterpret_cast<const sockaddr_in6*>(&addr);
        *out = SocketAddress(IPAddress(saddr->sin6_addr),
                network_to_host16(saddr->sin6_port));
        out->set_scope_ID(saddr->sin6_scope_id);
        return true;
    }
    return false;
}

} // namespace rtcbase


