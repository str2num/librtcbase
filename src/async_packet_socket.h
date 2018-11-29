/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
 
/**
 * @file async_packet_socket.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_ASYNC_PACKET_SOCKET_H_
#define  __RTCBASE_ASYNC_PACKET_SOCKET_H_

#include "constructor_magic.h"
#include "dscp.h"
#include "sigslot.h"
#include "socket.h"
#include "time_utils.h"

namespace rtcbase {

// This structure holds the info needed to update the packet send time header
// extension, including the information needed to update the authentication tag
// after changing the value.
struct PacketTimeUpdateParams {
    PacketTimeUpdateParams();
    ~PacketTimeUpdateParams();

    int rtp_sendtime_extension_id;    // extension header id present in packet.
    std::vector<char> srtp_auth_key;  // Authentication key.
    int srtp_auth_tag_len;            // Authentication tag length.
    int64_t srtp_packet_index;        // Required for Rtp Packet authentication.
};

// This structure holds meta information for the packet which is about to send
// over network.
struct PacketOptions {
    PacketOptions() : dscp(DSCP_NO_CHANGE), packet_id(-1) {}
    explicit PacketOptions(DiffServCodePoint dscp) : dscp(dscp), packet_id(-1) {}

    DiffServCodePoint dscp;
    int packet_id;  // 16 bits, -1 represents "not set".
    PacketTimeUpdateParams packet_time_params;
};

// This structure will have the information about when packet is actually
// received by socket.
struct PacketTime {
    PacketTime() : timestamp(-1), not_before(-1) {}
    PacketTime(int64_t timestamp, int64_t not_before)
        : timestamp(timestamp), not_before(not_before) {}

    int64_t timestamp;   // Receive time after socket delivers the data.

    // Earliest possible time the data could have arrived, indicating the
    // potential error in the |timestamp| value, in case the system, is busy. For
    // example, the time of the last select() call.
    // If unknown, this value will be set to zero.
    int64_t not_before;
};

inline PacketTime create_packet_time(int64_t not_before) {
    return PacketTime(time_micros(), not_before);
}

// Provides the ability to receive packets asynchronously. Sends are not
// buffered since it is acceptable to drop packets under high load.
class AsyncPacketSocket : public HasSlots<> {
public:
    enum State {
        STATE_CLOSED,
        STATE_BINDING,
        STATE_BOUND,
        STATE_CONNECTING,
        STATE_CONNECTED
    };

    AsyncPacketSocket();
    ~AsyncPacketSocket() override;
    
    virtual SocketAddress get_local_address() const = 0;

    // Send a packet.
    virtual int send_to(const void *pv, size_t cb, const SocketAddress& addr,
            const PacketOptions& options) = 0;

    // Close the socket.
    virtual int close() = 0;

    // Returns current state of the socket.
    virtual State get_state() const = 0;

    // Get/set options.
    virtual int get_option(Socket::Option opt, int* value) = 0;
    virtual int set_option(Socket::Option opt, int value) = 0;

    virtual int get_error() const = 0;

    // Emitted each time a packet is read. Used only for UDP and
    // connected TCP sockets.
    rtcbase::Signal5<AsyncPacketSocket*, const char*, size_t,
        const SocketAddress&,
        const PacketTime&> signal_read_packet;

    // Emitted each time a packet is sent.
    rtcbase::Signal2<AsyncPacketSocket*, const SentPacket&> signal_sent_packet;
};

} // namespace rtcbase

#endif  //__RTCBASE_ASYNC_PACKET_SOCKET_H_


