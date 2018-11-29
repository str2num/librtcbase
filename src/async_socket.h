/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
 
/**
 * @file async_socket.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_ASYNC_SOCKET_H_
#define  __RTCBASE_ASYNC_SOCKET_H_

#include "sigslot.h"
#include "socket.h"

namespace rtcbase {

// Provides the ability to perform socket I/O asynchronously.
class AsyncSocket : public Socket {
public:
    AsyncSocket();
    ~AsyncSocket() override;
    
    // SignalReadEvent and SignalWriteEvent use multi_threaded_local to allow
    // access concurrently from different thread.
    // For example SignalReadEvent::connect will be called in AsyncUDPSocket ctor
    // but at the same time the SocketDispatcher maybe signaling the read event.
    // ready to read
    Signal1<AsyncSocket*, MultiThreadedLocal> signal_read_event;
    // ready to write
    Signal1<AsyncSocket*, MultiThreadedLocal> signal_write_event;
};

} // namespace rtcbase

#endif  //__RTCBASE_ASYNC_SOCKET_H_


