/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file socket_address.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SOCKET_ADDRESS_H_
#define  __RTCBASE_SOCKET_ADDRESS_H_

#include <string>
#include <vector>
#include <iosfwd>
#include "basic_types.h"
#include "ipaddress.h"

#undef set_port

struct sockaddr_in;
struct sockaddr_storage;

namespace rtcbase {

// Records an IP address and port.
class SocketAddress {
public:
    // Creates a nil address.
    SocketAddress();

    // Creates the address with the given host and port. Host may be a
    // literal IP string or a hostname to be resolved later.
    SocketAddress(const std::string& hostname, int port);

    // Creates the address with the given IP and port.
    // IP is given as an integer in host byte order. V4 only, to be deprecated.
    SocketAddress(uint32_t ip_as_host_order_integer, int port);

    // Creates the address with the given IP and port.
    SocketAddress(const IPAddress& ip, int port);

    // Creates a copy of the given address.
    SocketAddress(const SocketAddress& addr);
    
    // Resets to the nil address.
    void clear();
    
    // Determines if this is a nil address (empty hostname, any IP, null port)
    bool is_nil() const; 
    
    // Replaces our address with the given one.
    SocketAddress& operator=(const SocketAddress& addr); 

    // Changes the IP of this address to the given one, and clears the hostname
    // IP is given as an integer in host byte order. V4 only, to be deprecated..
    void set_IP(uint32_t ip_as_host_order_integer);

    // Changes the IP of this address to the given one, and clears the hostname.
    void set_IP(const IPAddress& ip);

    // Changes the hostname of this address to the given one.
    // Does not resolve the address; use Resolve to do so.
    void set_IP(const std::string& hostname);
    
    // Changes the port of this address to the given one.
    void set_port(int port);
    
    const IPAddress& ipaddr() const;

    int family() const {return _ip.family(); }
    
    // Returns the port part of this address.
    uint16_t port() const;

    // Returns the scope ID associated with this address. Scope IDs are a
    // necessary addition to IPv6 link-local addresses, with different network
    // interfaces having different scope-ids for their link-local addresses.
    // IPv4 address do not have scope_ids and sockaddr_in structures do not have
    // a field for them.
    int scope_id() const {return _scope_id; }
    void set_scope_ID(int id) { _scope_id = id; }
    
    // Returns the 'host' portion of the address (hostname or IP) in a form
    // suitable for use in a URI. If both IP and hostname are present, hostname
    // is preferred. IPv6 addresses are enclosed in square brackets ('[' and ']').
    std::string host_as_URI_string() const;
    
    std::string port_as_string() const;

    // Same as HostAsURIString but anonymizes IP addresses by hiding the last
    // part.
    std::string host_as_sensitive_URI_string() const;

    // Returns hostname:port or [hostname]:port.
    std::string to_string() const;

    // Same as ToString but anonymizes it by hiding the last part.
    std::string to_sensitive_string() const;
    
    // Parses hostname:port and [hostname]:port.
    bool from_string(const std::string& str);

    friend std::ostream& operator<<(std::ostream& os, const SocketAddress& addr);
    
    // Determines whether this represents a missing / any IP address.
    // That is, 0.0.0.0 or ::.
    // Hostname and/or port may be set.
    bool is_any_IP() const;
    
    // Determines whether the IP address is in one of the private ranges:
    // For v4: 127.0.0.0/8 10.0.0.0/8 192.168.0.0/16 172.16.0.0/12.
    // For v6: FE80::/16 and ::1.
    bool is_private_IP() const; 
    
    // Determines whether the hostname has been resolved to an IP.
    bool is_unresolved_IP() const;

    // Determines whether this address is identical to the given one.
    bool operator ==(const SocketAddress& addr) const;
    inline bool operator !=(const SocketAddress& addr) const {
        return !this->operator ==(addr);
    }
    
    // Compares based on IP and then port.
    bool operator <(const SocketAddress& addr) const;

    // Determines whether this address has the same IP as the one given.
    bool equal_IPs(const SocketAddress& addr) const;

    // Determines whether this address has the same port as the one given.
    bool equal_ports(const SocketAddress& addr) const;

    // Read and write the address to/from a sockaddr_storage.
    // Dual stack version always sets family to AF_INET6, and maps v4 addresses.
    // The other version doesn't map, and outputs an AF_INET address for
    // v4 or mapped addresses, and AF_INET6 addresses for others.
    // Returns the size of the sockaddr_in or sockaddr_in6 structure that is
    // written to the sockaddr_storage, or zero on failure.
    //size_t ToDualStackSockAddrStorage(sockaddr_storage* saddr) const;
    size_t to_sockaddr_storage(sockaddr_storage* saddr) const;

private:
    std::string _hostname;
    IPAddress _ip;
    uint16_t _port;
    int _scope_id;
    bool _literal;  // Indicates that 'hostname_' contains a literal IP string.
};

bool socket_address_from_sockaddr_storage(const sockaddr_storage& saddr,
        SocketAddress* out);

} // namespace rtcbase

#endif  //__RTCBASE_SOCKET_ADDRESS_H_


