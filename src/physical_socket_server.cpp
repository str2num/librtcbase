/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file physical_socket_server.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/tcp.h>  // for TCP_NODELAY

#include "logging.h"
#include "time_utils.h"
#include "physical_socket_server.h"

typedef void* SockOptArg;

namespace rtcbase {

int64_t get_socket_recv_timestamp(int socket) {
    struct timeval tv_ioctl;
    int ret = ioctl(socket, SIOCGSTAMP, &tv_ioctl);
    if (ret != 0) {
        return -1;
    }
    int64_t timestamp =
        rtcbase::k_num_microsecs_per_sec * static_cast<int64_t>(tv_ioctl.tv_sec) +
        static_cast<int64_t>(tv_ioctl.tv_usec);
    return timestamp;
}

///////////////// PhysicalSocketServer //////////////////

PhysicalSocketServer::PhysicalSocketServer() {}

PhysicalSocketServer::~PhysicalSocketServer() {}

Socket* PhysicalSocketServer::create_socket(int family, int type) {
    PhysicalSocket* socket = new PhysicalSocket(this);
    if (socket->create(family, type)) {
        return socket;
    } else {
        delete socket;
        return nullptr;
    }
}

AsyncSocket* PhysicalSocketServer::create_async_socket(int family, int type) {
    PhysicalSocket* socket = new PhysicalSocket(this);
    if (socket->create_async(family, type)) {
        return socket;
    } else {
        delete socket;
        return nullptr;
    }
}

/////////////////// PhysicalSocket //////////////////

PhysicalSocket::PhysicalSocket(PhysicalSocketServer* ss, SOCKET s) : 
    MemCheck("PhysicalSocket"), _ss(ss), _s(s), _error(0)
{
    if (_s != INVALID_SOCKET) {
        int type = SOCK_STREAM;
        socklen_t len = sizeof(type);
        if (getsockopt(_s, SOL_SOCKET, SO_TYPE, (SockOptArg)&type, &len) != 0) {
            LOG(LS_WARNING) << "getsockopt failed, socket: " << _s;
        }
        _udp = (SOCK_DGRAM == type);
    }
}

PhysicalSocket::~PhysicalSocket() {
    close();
}

bool PhysicalSocket::create(int family, int type) {
    close();
    _s = ::socket(family, type, 0);
    _udp = (SOCK_DGRAM == type);
    update_last_error();
    return _s != INVALID_SOCKET;
}

bool PhysicalSocket::create_async(int family, int type) {
    if (!create(family, type)) {
        return false;
    }

    fcntl(_s, F_SETFL, fcntl(_s, F_GETFL, 0) | O_NONBLOCK);
    return true;
}

SocketAddress PhysicalSocket::get_local_address() const {
    sockaddr_storage addr_storage;
    socklen_t addrlen = sizeof(addr_storage);
    sockaddr* addr = reinterpret_cast<sockaddr*>(&addr_storage);
    int result = ::getsockname(_s, addr, &addrlen);
    SocketAddress address;
    if (result >= 0) {
        socket_address_from_sockaddr_storage(addr_storage, &address);
    } else {
        LOG(LS_WARNING) << "GetLocalAddress: unable to get local addr, socket="
            << _s;
    }
    return address;
}

int PhysicalSocket::bind(const SocketAddress& bind_addr) {
    sockaddr_storage addr_storage;
    size_t len = bind_addr.to_sockaddr_storage(&addr_storage);
    sockaddr* addr = reinterpret_cast<sockaddr*>(&addr_storage);
    int err = ::bind(_s, addr, static_cast<int>(len));
    update_last_error();
    return err;
}

int PhysicalSocket::send_to(const void* buffer,
        size_t length,
        const SocketAddress& addr) 
{
    sockaddr_storage saddr;
    size_t len = addr.to_sockaddr_storage(&saddr);
    int sent = do_send_to(
            _s, static_cast<const char *>(buffer), static_cast<int>(length),
            MSG_NOSIGNAL, // Suppress SIGPIPE. See above for explanation.
            reinterpret_cast<sockaddr*>(&saddr), static_cast<int>(len));
    update_last_error();
    
    if (sent < 0) {
        if (is_blocking_error(get_error())) {
            sent = 0;
        } else {
            sent = -1; // error
            LOG(LS_WARNING) << "Send data error, addr: " << addr.to_string() << ", socket: " << _s;
        }
    } else if (sent == 0) {
        sent = -1;
        LOG(LS_WARNING) << "Send data error, addr: " << addr.to_string() << ", socket: " << _s;
    }
    return sent;
}

int PhysicalSocket::recv_from(void* buffer,
        size_t length,
        SocketAddress* out_addr,
        int64_t* timestamp) 
{
    sockaddr_storage addr_storage;
    socklen_t addr_len = sizeof(addr_storage);
    sockaddr* addr = reinterpret_cast<sockaddr*>(&addr_storage);
    int received = ::recvfrom(_s, static_cast<char*>(buffer),
            static_cast<int>(length), 0, addr, &addr_len);
    if (timestamp) {
        *timestamp = get_socket_recv_timestamp(_s);
    }
    update_last_error();
    if ((received >= 0) && (out_addr != nullptr)) {
        socket_address_from_sockaddr_storage(addr_storage, out_addr);
    }
    int error = get_error();
    
    if (received < 0) {
        if (is_blocking_error(error)) {
            received = 0;
        } else {
            received = -1; // error
            LOG(LS_WARNING) << "Read data error, socket: " << _s;
        }
    } else if (received == 0) {
        received = -1;
        LOG(LS_WARNING) << "Read data error, socket: " << _s;
    }
    
    return received;
}

int PhysicalSocket::close() {
    if (_s == INVALID_SOCKET) {
        return 0;
    }
    int err = ::closesocket(_s);
    update_last_error();
    _s = INVALID_SOCKET;
    return err;
}

int PhysicalSocket::get_error() const {
    return _error;
}

void PhysicalSocket::set_error(int error) {
    _error = error;
}

int PhysicalSocket::get_option(Option opt, int* value) {
    int slevel;
    int sopt;
    if (translate_option(opt, &slevel, &sopt) == -1) {
        return -1;
    }
    socklen_t optlen = sizeof(*value);
    int ret = ::getsockopt(_s, slevel, sopt, (SockOptArg)value, &optlen);
    if (ret != -1 && opt == OPT_DONTFRAGMENT) {
        *value = (*value != IP_PMTUDISC_DONT) ? 1 : 0;
    }
    return ret;
}

int PhysicalSocket::set_option(Option opt, int value) {
    int slevel;
    int sopt;
    if (translate_option(opt, &slevel, &sopt) == -1) {
        return -1;
    }
    if (opt == OPT_DONTFRAGMENT) {
        value = (value) ? IP_PMTUDISC_DO : IP_PMTUDISC_DONT;
    }
    return ::setsockopt(_s, slevel, sopt, (SockOptArg)&value, sizeof(value));
}

int PhysicalSocket::do_send_to(SOCKET socket,
        const char* buf,
        int len,
        int flags,
        const struct sockaddr* dest_addr,
        socklen_t addrlen) 
{
    return ::sendto(socket, buf, len, flags, dest_addr, addrlen);
}

void PhysicalSocket::update_last_error() {
    set_error(LAST_SYSTEM_ERROR);
}

int PhysicalSocket::translate_option(Option opt, int* slevel, int* sopt) {
    switch (opt) {
        case OPT_DONTFRAGMENT:
            *slevel = IPPROTO_IP;
            *sopt = IP_MTU_DISCOVER;
            break;
        case OPT_RCVBUF:
            *slevel = SOL_SOCKET;
            *sopt = SO_RCVBUF;
            break;
        case OPT_SNDBUF:
            *slevel = SOL_SOCKET;
            *sopt = SO_SNDBUF;
            break;
        case OPT_NODELAY:
            *slevel = IPPROTO_TCP;
            *sopt = TCP_NODELAY;
            break;
        case OPT_DSCP:
            LOG(LS_WARNING) << "Socket::OPT_DSCP not supported.";
            return -1;
        case OPT_RTP_SENDTIME_EXTN_ID:
            return -1;  // No logging is necessary as this not a OS socket option.
        default:
            LOG(LS_WARNING) << "Socket::" << opt << " not supported";
            return -1;
    }
    return 0;
}

} // namespace rtcbase


