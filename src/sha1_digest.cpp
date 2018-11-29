/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file sha1_digest.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "sha1_digest.h"

namespace rtcbase {

size_t Sha1Digest::size() const {
    return k_size;
}

void Sha1Digest::update(const void* buf, size_t len) {
    SHA1_update(&_ctx, static_cast<const uint8_t*>(buf), len);
}

size_t Sha1Digest::finish(void* buf, size_t len) {
    if (len < k_size) {
        return 0;
    }
    SHA1_final(&_ctx, static_cast<uint8_t*>(buf));
    SHA1_init(&_ctx);  // Reset for next use.
    return k_size;
}

}  // namespace rtcbase


