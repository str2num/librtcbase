/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file socket_factory.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SOCKET_FACTORY_H_
#define  __RTCBASE_SOCKET_FACTORY_H_

#include "socket.h"
#include "async_socket.h"

namespace rtcbase {

class SocketFactory {
public:
    virtual ~SocketFactory() {}

    // Returns a new socket for blocking communication.  The type can be
    // SOCK_DGRAM and SOCK_STREAM.
    virtual Socket* create_socket(int family, int type) = 0;
    // Returns a new socket for nonblocking communication.  The type can be
    // SOCK_DGRAM and SOCK_STREAM.
    virtual AsyncSocket* create_async_socket(int family, int type) = 0;
};

} // namespace rtcbase

#endif  //__RTCBASE_SOCKET_FACTORY_H_


