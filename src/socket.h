/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file socket.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SOCKET_H_
#define  __RTCBASE_SOCKET_H_

#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define SOCKET_EACCES EACCES

#include "basic_types.h"
#include "constructor_magic.h"
#include "socket_address.h"

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket(s) close(s)

namespace rtcbase {

inline bool is_blocking_error(int e) {
    return (e == EWOULDBLOCK) || (e == EAGAIN) || (e == EINPROGRESS);
}

struct SentPacket {
    SentPacket() : packet_id(-1), send_time_ms(-1) {}
    SentPacket(int packet_id, int64_t send_time_ms)
        : packet_id(packet_id), send_time_ms(send_time_ms) {}

    int packet_id;
    int64_t send_time_ms;
};

// General interface for the socket implementations of various networks.  The
// methods match those of normal UNIX sockets very closely.
class Socket {
public:
    virtual ~Socket() {}
    
    // Returns the address to which the socket is bound.  If the socket is not
    // bound, then the any-address is returned.
    virtual SocketAddress get_local_address() const = 0;

    // Returns the address to which the socket is connected.  If the socket is
    // not connected, then the any-address is returned.
    //virtual SocketAddress get_remote_address() const = 0;

    virtual int bind(const SocketAddress& addr) = 0;
    //virtual int Connect(const SocketAddress& addr) = 0;
    //virtual int Send(const void *pv, size_t cb) = 0;
    virtual int send_to(const void* pv, size_t cb, const SocketAddress& addr) = 0;
    // |timestamp| is in units of microseconds.
    //virtual int Recv(void* pv, size_t cb, int64_t* timestamp) = 0;
    virtual int recv_from(void* pv,
            size_t cb,
            SocketAddress* paddr,
            int64_t* timestamp) = 0;
    /*
    virtual int Listen(int backlog) = 0;
    virtual Socket *Accept(SocketAddress *paddr) = 0;
    */
    virtual int close() = 0;
    virtual int get_error() const = 0;
    virtual void set_error(int error) = 0;
    /*
    inline bool IsBlocking() const { return IsBlockingError(GetError()); }

    enum ConnState {
        CS_CLOSED,
        CS_CONNECTING,
        CS_CONNECTED
    };
    virtual ConnState GetState() const = 0;

    // Fills in the given uint16_t with the current estimate of the MTU along the
    // path to the address to which this socket is connected. NOTE: This method
    // can block for up to 10 seconds on Windows.
    virtual int EstimateMTU(uint16_t* mtu) = 0;
    */

    enum Option {
        OPT_DONTFRAGMENT,
        OPT_RCVBUF,      // receive buffer size
        OPT_SNDBUF,      // send buffer size
        OPT_NODELAY,     // whether Nagle algorithm is enabled
        OPT_IPV6_V6ONLY, // Whether the socket is IPv6 only.
        OPT_DSCP,        // DSCP code
        OPT_RTP_SENDTIME_EXTN_ID,  // This is a non-traditional socket option param.
                                   // This is specific to libjingle and will be used
                                   // if SendTime option is needed at socket level.
    };
    
    virtual int get_option(Option opt, int* value) = 0;
    virtual int set_option(Option opt, int value) = 0;
    
    virtual int get_fd() = 0;

protected:
    Socket() {}

private:
    RTC_DISALLOW_COPY_AND_ASSIGN(Socket);
};

}  // namespace rtcbase

#endif  //__RTCBASE_SOCKET_H_


