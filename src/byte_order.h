/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file byte_order.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_BYTE_ORDER_H_
#define  __RTCBASE_BYTE_ORDER_H_

#include <arpa/inet.h>

namespace rtcbase {

// Reading and writing of little and big-endian numbers from memory
// TODO: Optimized versions, with direct read/writes of
// integers in host-endian format, when the platform supports it.

inline void set8(void* memory, size_t offset, uint8_t v) {
    static_cast<uint8_t*>(memory)[offset] = v;
}

inline uint8_t get8(const void* memory, size_t offset) {
    return static_cast<const uint8_t*>(memory)[offset];
}

inline void set_be16(void* memory, uint16_t v) {
    set8(memory, 0, static_cast<uint8_t>(v >> 8));
    set8(memory, 1, static_cast<uint8_t>(v >> 0));
}

inline void set_be32(void* memory, uint32_t v) {
    set8(memory, 0, static_cast<uint8_t>(v >> 24));
    set8(memory, 1, static_cast<uint8_t>(v >> 16));
    set8(memory, 2, static_cast<uint8_t>(v >> 8));
    set8(memory, 3, static_cast<uint8_t>(v >> 0));
}

inline void set_be64(void* memory, uint64_t v) {
    set8(memory, 0, static_cast<uint8_t>(v >> 56));
    set8(memory, 1, static_cast<uint8_t>(v >> 48));
    set8(memory, 2, static_cast<uint8_t>(v >> 40));
    set8(memory, 3, static_cast<uint8_t>(v >> 32));
    set8(memory, 4, static_cast<uint8_t>(v >> 24));
    set8(memory, 5, static_cast<uint8_t>(v >> 16));
    set8(memory, 6, static_cast<uint8_t>(v >> 8));
    set8(memory, 7, static_cast<uint8_t>(v >> 0));
}

inline uint16_t get_be16(const void* memory) {
    return static_cast<uint16_t>((get8(memory, 0) << 8) | (get8(memory, 1) << 0));
}

inline uint32_t get_be32(const void* memory) {
    return (static_cast<uint32_t>(get8(memory, 0)) << 24) |
        (static_cast<uint32_t>(get8(memory, 1)) << 16) |
        (static_cast<uint32_t>(get8(memory, 2)) << 8) |
        (static_cast<uint32_t>(get8(memory, 3)) << 0);
}

inline uint64_t get_be64(const void* memory) {
    return (static_cast<uint64_t>(get8(memory, 0)) << 56) |
        (static_cast<uint64_t>(get8(memory, 1)) << 48) |
        (static_cast<uint64_t>(get8(memory, 2)) << 40) |
        (static_cast<uint64_t>(get8(memory, 3)) << 32) |
        (static_cast<uint64_t>(get8(memory, 4)) << 24) |
        (static_cast<uint64_t>(get8(memory, 5)) << 16) |
        (static_cast<uint64_t>(get8(memory, 6)) << 8) |
        (static_cast<uint64_t>(get8(memory, 7)) << 0);
}

inline void set_le16(void* memory, uint16_t v) {
    set8(memory, 0, static_cast<uint8_t>(v >> 0));
    set8(memory, 1, static_cast<uint8_t>(v >> 8));
}

inline void set_le32(void* memory, uint32_t v) {
    set8(memory, 0, static_cast<uint8_t>(v >> 0));
    set8(memory, 1, static_cast<uint8_t>(v >> 8));
    set8(memory, 2, static_cast<uint8_t>(v >> 16));
    set8(memory, 3, static_cast<uint8_t>(v >> 24));
}

inline void set_le64(void* memory, uint64_t v) {
    set8(memory, 0, static_cast<uint8_t>(v >> 0));
    set8(memory, 1, static_cast<uint8_t>(v >> 8));
    set8(memory, 2, static_cast<uint8_t>(v >> 16));
    set8(memory, 3, static_cast<uint8_t>(v >> 24));
    set8(memory, 4, static_cast<uint8_t>(v >> 32));
    set8(memory, 5, static_cast<uint8_t>(v >> 40));
    set8(memory, 6, static_cast<uint8_t>(v >> 48));
    set8(memory, 7, static_cast<uint8_t>(v >> 56));
}

inline uint16_t get_le16(const void* memory) {
    return static_cast<uint16_t>((get8(memory, 0) << 0) | (get8(memory, 1) << 8));
}

inline uint32_t get_le32(const void* memory) {
    return (static_cast<uint32_t>(get8(memory, 0)) << 0) |
        (static_cast<uint32_t>(get8(memory, 1)) << 8) |
        (static_cast<uint32_t>(get8(memory, 2)) << 16) |
        (static_cast<uint32_t>(get8(memory, 3)) << 24);
}

inline uint64_t get_le64(const void* memory) {
    return (static_cast<uint64_t>(get8(memory, 0)) << 0) |
        (static_cast<uint64_t>(get8(memory, 1)) << 8) |
        (static_cast<uint64_t>(get8(memory, 2)) << 16) |
        (static_cast<uint64_t>(get8(memory, 3)) << 24) |
        (static_cast<uint64_t>(get8(memory, 4)) << 32) |
        (static_cast<uint64_t>(get8(memory, 5)) << 40) |
        (static_cast<uint64_t>(get8(memory, 6)) << 48) |
        (static_cast<uint64_t>(get8(memory, 7)) << 56);
}

// Check if the current host is big endian.
inline bool is_host_big_endian() {
    static const int number = 1;
    return 0 == *reinterpret_cast<const char*>(&number);
}

inline uint16_t host_to_network16(uint16_t n) {
    uint16_t result;
    set_be16(&result, n);
    return result;
}

inline uint32_t host_to_network32(uint32_t n) {
    uint32_t result;
    set_be32(&result, n);
    return result;
}

inline uint64_t host_to_network64(uint64_t n) {
    uint64_t result;
    set_be64(&result, n);
    return result;
}

inline uint16_t network_to_host16(uint16_t n) {
    return get_be16(&n);
}

inline uint32_t network_to_host32(uint32_t n) {
    return get_be32(&n);
}

inline uint64_t network_to_host64(uint64_t n) {
    return get_be64(&n);
}

}  // namespace rtcbase

#endif  //__RTCBASE_BYTE_ORDER_H_


