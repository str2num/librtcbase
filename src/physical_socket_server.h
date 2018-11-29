/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file physical_socket_server.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_PHYSICAL_SOCKET_SERVER_H_
#define  __RTCBASE_PHYSICAL_SOCKET_SERVER_H_

#include "memcheck.h"
#include "socket_factory.h"
#include "async_socket.h"

typedef int SOCKET;

namespace rtcbase {

class PhysicalSocketServer : public SocketFactory {
public:
    PhysicalSocketServer();
    ~PhysicalSocketServer() override;

    // SocketFactory
    Socket* create_socket(int family, int type) override;
    AsyncSocket* create_async_socket(int family, int type) override;
};

class PhysicalSocket : public AsyncSocket, public HasSlots<>, public MemCheck {
public:
    PhysicalSocket(PhysicalSocketServer* ss, SOCKET s = INVALID_SOCKET);
    ~PhysicalSocket() override;

    // Creates the underlying OS socket (same as the "socket" function).
    virtual bool create(int family, int type);
    virtual bool create_async(int family, int type);
    
    SocketAddress get_local_address() const override;

    int bind(const SocketAddress& bind_addr) override;

    int get_error() const override;
    void set_error(int error) override;
    
    int get_option(Option opt, int* value) override;
    int set_option(Option opt, int value) override;
    
    //int send(const void* pv, size_t cb) override;
    int send_to(const void* buffer,
            size_t length,
            const SocketAddress& addr) override;
    
    int recv_from(void* buffer,
            size_t length,
            SocketAddress* out_addr,
            int64_t* timestamp) override;
    int close() override;
    
    SOCKET get_fd() { return _s; } 

protected:
    virtual int do_send_to(SOCKET socket, const char* buf, int len, int flags,
            const struct sockaddr* dest_addr, socklen_t addrlen);

    void update_last_error();
    
    static int translate_option(Option opt, int* slevel, int* sopt);

private:
    PhysicalSocketServer* _ss;
    SOCKET _s;
    bool _udp;
    int _error;
};

} // namespace rtcbase

#endif  //__RTCBASE_PHYSICAL_SOCKET_SERVER_H_


