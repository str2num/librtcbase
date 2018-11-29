/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file async_udp_socket.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <assert.h>

#include "logging.h"
#include "event_loop.h"
#include "async_udp_socket.h"

namespace rtcbase {

void socket_io_cb(EventLoop *el, IOWatcher *w, int fd, int revents, void* data) {
    (void)el;
    (void)w;

    if (!data) {
        LOG(LS_WARNING) << "Ptr data is NULL";
        return;
    }
    
    AsyncUDPSocket* udp_socket = (AsyncUDPSocket*)data;
    if (revents & EventLoop::READ) {
        udp_socket->recv_data(fd);
    } 
    
    if (revents & EventLoop::WRITE) {
        udp_socket->send_data();
    }
}

static const int BUF_SIZE = 64 * 1024;

AsyncUDPSocket::AsyncUDPSocket(EventLoop* el, AsyncSocket* socket)
    : _socket(socket), _total_send_data(0) 
{
    assert(el);
    _el = el;

    _size = BUF_SIZE;
    _buf = new char[_size];
 
    _socket_watcher = _el->create_io_event(socket_io_cb, (void*)this);
    enable_events(EventLoop::READ);
}

AsyncUDPSocket::~AsyncUDPSocket() {
    if (_socket_watcher) {
        _el->delete_io_event(_socket_watcher);
        _socket_watcher = NULL;
    }
    delete [] _buf;

    UdpDataSendMap::iterator iter = _udp_data_send_map.begin();
    for (; iter != _udp_data_send_map.end(); ++iter) {
        if (iter->second.packet_list.empty()) {
            continue;
        }
        
        std::list<UdpPacketData*>::iterator it = iter->second.packet_list.begin();
        for (; it != iter->second.packet_list.end(); ++it) {
            delete *it;
        }
    }
}

void AsyncUDPSocket::enable_events(int events) {
    _el->start_io_event(_socket_watcher, _socket->get_fd(), events);
}

void AsyncUDPSocket::disable_events(int events) {
    _el->stop_io_event(_socket_watcher, _socket->get_fd(), events);
}

SocketAddress AsyncUDPSocket::get_local_address() const {
    return _socket->get_local_address();
}

void AsyncUDPSocket::send_data() { 
    while (_total_send_data > 0) {
        UdpDataSendMap::iterator iter = _udp_data_send_map.begin();
        for (; iter != _udp_data_send_map.end(); ++iter) {
            if (iter->second.packet_list.empty()) {
                continue;
            }
            UdpPacketData* packet_data = iter->second.packet_list.front();
            int sent = _socket->send_to(packet_data->data(), 
                    packet_data->size(), 
                    packet_data->addr());
            if (-1 == sent) {
                LOG(LS_WARNING) << "Send data return error";
                delete iter->second.packet_list.front();
                iter->second.packet_list.pop_front();
                --_total_send_data;
                return;
            } else if (0 == sent) {
                LOG(LS_WARNING) << "Write zere bytes, addr: " << packet_data->addr().to_string();
                return;
            } else { // finish
                delete iter->second.packet_list.front();
                iter->second.packet_list.pop_front();
                --_total_send_data;
                rtcbase::SentPacket sent_packet(-1, rtcbase::time_millis());
                signal_sent_packet(this, sent_packet);
            } 
        }
    }

    if (_total_send_data <= 0) {
        disable_events(EventLoop::WRITE);
    }
}

int AsyncUDPSocket::add_udp_send_data(const void* data, size_t size,
        const SocketAddress& addr)
{
    UdpPacketData* packet_data = new UdpPacketData(data, size, addr);
    UdpDataSendMap::iterator iter;
    std::string key = addr.to_string();
    iter = _udp_data_send_map.find(key);
    if (iter != _udp_data_send_map.end()) {
        iter->second.packet_list.push_back(packet_data);
    } else {
        _udp_data_send_map[key].packet_list.push_back(packet_data);
    }
    ++_total_send_data;
    enable_events(EventLoop::WRITE);
    return size;
}

int AsyncUDPSocket::send_to(const void *data, size_t size,
        const SocketAddress& addr,
        const rtcbase::PacketOptions& options) 
{ 
    (void)options;
    return add_udp_send_data(data, size, addr);    
}

void AsyncUDPSocket::recv_data(int fd) {
    assert(_socket->get_fd() == fd);

    SocketAddress remote_addr;
    int64_t timestamp;
    int len = _socket->recv_from(_buf, _size, &remote_addr, &timestamp);
    SocketAddress local_addr = _socket->get_local_address();
    if (len < 0) {
        // An error here typically means we got an ICMP error in response to our
        // send datagram, indicating the remote address was unreachable.
        // When doing ICE, this kind of thing will often happen.
        // TODO: Do something better like forwarding the error to the user.
        LOG(LS_WARNING) << "AsyncUDPSocket[" << local_addr.to_sensitive_string() << "] "
            << "receive failed with error " << _socket->get_error();
        return;
    } else if (0 == len) {
        LOG(LS_WARNING) << "AsyncUDPSocket[" << local_addr.to_sensitive_string() << "] "
            << "receive zero bytes";
        return;
    }

    // TODO: Make sure that we got all of the packet.
    // If we did not, then we should resize our buffer to be large enough.
    signal_read_packet(
            this, _buf, static_cast<size_t>(len), remote_addr,
            (timestamp > -1 ? PacketTime(timestamp, 0) : create_packet_time(0))); 
}

int AsyncUDPSocket::close() {
    disable_events(EventLoop::READ | EventLoop::WRITE); 
    return _socket->close();
}

AsyncUDPSocket::State AsyncUDPSocket::get_state() const {
    return STATE_BOUND;
}

int AsyncUDPSocket::get_option(Socket::Option opt, int* value) {
    return _socket->get_option(opt, value);
}

int AsyncUDPSocket::set_option(Socket::Option opt, int value) {
    return _socket->set_option(opt, value);
}

int AsyncUDPSocket::get_error() const {
    return _socket->get_error();
}

} // namespace rtcbase


