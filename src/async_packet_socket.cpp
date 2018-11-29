/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
 
/**
 * @file async_packet_socket.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "async_packet_socket.h"

namespace rtcbase {

PacketTimeUpdateParams::PacketTimeUpdateParams()
    : rtp_sendtime_extension_id(-1),
    srtp_auth_tag_len(-1),
    srtp_packet_index(-1) {}

PacketTimeUpdateParams::~PacketTimeUpdateParams() = default;

AsyncPacketSocket::AsyncPacketSocket() {}

AsyncPacketSocket::~AsyncPacketSocket() {}

}  // namespace rtcbase


