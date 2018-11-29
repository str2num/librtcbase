/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file crc32.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "array_size.h"
#include "crc32.h"

namespace rtcbase {

// This implementation is based on the sample implementation in RFC 1952.

// CRC32 polynomial, in reversed form.
// See RFC 1952, or http://en.wikipedia.org/wiki/Cyclic_redundancy_check
static const uint32_t k_crc32_polynomial = 0xEDB88320;
static uint32_t k_crc32_table[256] = {0};

static void ensure_crc32_table_inited() {
    if (k_crc32_table[arraysize(k_crc32_table) - 1]) {
        return;  // already inited
    }
    for (uint32_t i = 0; i < arraysize(k_crc32_table); ++i) {
        uint32_t c = i;
        for (size_t j = 0; j < 8; ++j) {
            if (c & 1) {
                c = k_crc32_polynomial ^ (c >> 1);
            } else {
                c >>= 1;
            }
        }
        k_crc32_table[i] = c;
    }
}

uint32_t update_crc32(uint32_t start, const void* buf, size_t len) {
    ensure_crc32_table_inited();

    uint32_t c = start ^ 0xFFFFFFFF;
    const uint8_t* u = static_cast<const uint8_t*>(buf);
    for (size_t i = 0; i < len; ++i) {
        c = k_crc32_table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}

}  // namespace rtcbase


