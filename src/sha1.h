/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
/**
 * @file sha1.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SHA1_H_
#define  __RTCBASE_SHA1_H_

#include <stdint.h>
#include <stdlib.h>

namespace rtcbase {

struct SHA1_CTX {
    uint32_t state[5];
    // TODO: Change bit count to uint64_t.
    uint32_t count[2];  // Bit count of input.
    uint8_t buffer[64];
};

#define SHA1_DIGEST_SIZE 20

void SHA1_init(SHA1_CTX* context);
void SHA1_update(SHA1_CTX* context, const uint8_t* data, size_t len);
void SHA1_final(SHA1_CTX* context, uint8_t digest[SHA1_DIGEST_SIZE]);

} // namespace rtcbase

#endif  //__RTCBASE_SHA1_H_


