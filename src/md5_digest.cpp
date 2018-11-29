/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file md5_digest.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "md5_digest.h"

namespace rtcbase {

size_t Md5Digest::size() const {
    return k_size;
}

void Md5Digest::update(const void* buf, size_t len) {
    MD5_update(&_ctx, static_cast<const uint8_t*>(buf), len);
}

size_t Md5Digest::finish(void* buf, size_t len) {
    if (len < k_size) {
        return 0;
    }
    MD5_final(&_ctx, static_cast<uint8_t*>(buf));
    MD5_init(&_ctx);  // Reset for next use.
    return k_size;
}

} // namespace rtcbase


