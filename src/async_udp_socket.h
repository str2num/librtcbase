/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
 
/**
 * @file async_udp_socket.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_ASYNC_UDP_SOCKET_H_
#define  __RTCBASE_ASYNC_UDP_SOCKET_H_

#include <memory>
#include <map>

#include "memcheck.h"
#include "async_packet_socket.h"
#include "socket_factory.h"
#include "event_loop.h"

namespace rtcbase {

class UdpPacketData : public MemCheck {
public:
    UdpPacketData(const void* data, size_t size, const SocketAddress& addr) :
        MemCheck("UdpPacketData"), _data(new char[size]), _size(size), _addr(addr)
    {
        memcpy(_data, data, size);
    }
     
    ~UdpPacketData() {
        if (_data) {
            delete _data;
            _data = NULL;    
        }
    }
    
    char* data() { return _data; }
    size_t size() { return _size; }
    SocketAddress addr() { return _addr; }

private:
    char* _data;
    size_t _size;
    SocketAddress _addr;
};

struct UdpPacketSendQueue {
    std::list<UdpPacketData*> packet_list;
};

typedef std::map<std::string, UdpPacketSendQueue> UdpDataSendMap;

// Provides the ability to receive packets asynchronously.  Sends are not
// buffered since it is acceptable to drop packets under high load.
class AsyncUDPSocket : public AsyncPacketSocket {
public:
    explicit AsyncUDPSocket(EventLoop* el, AsyncSocket* socket);
    ~AsyncUDPSocket() override;

    void enable_events(int events);
    void disable_events(int events);

    SocketAddress get_local_address() const override;

    int send_to(const void* data,
            size_t size,
            const SocketAddress& addr,
            const rtcbase::PacketOptions& options) override;
    int close() override;

    State get_state() const override;
    int get_option(Socket::Option opt, int* value) override;
    int set_option(Socket::Option opt, int value) override;

    int get_error() const override;

    void send_data();
    void recv_data(int fd);

protected:
    int add_udp_send_data(const void* data, size_t size, const SocketAddress& addr);

private:
    std::unique_ptr<AsyncSocket> _socket;
    char* _buf;
    size_t _size;

    EventLoop* _el;
    IOWatcher* _socket_watcher;

    UdpDataSendMap _udp_data_send_map;
    int _total_send_data;
};

}  // namespace rtcbase

#endif  //__RTCBASE_ASYNC_UDP_SOCKET_H_


