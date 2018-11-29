/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file sha1_digest.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SHA1_DIGEST_H_
#define  __RTCBASE_SHA1_DIGEST_H_

#include "message_digest.h"
#include "sha1.h"

namespace rtcbase {

// A simple wrapper for our SHA-1 implementation.
class Sha1Digest : public MessageDigest {
public:
    enum { k_size = SHA1_DIGEST_SIZE };
    Sha1Digest() {
        SHA1_init(&_ctx);
    }
    size_t size() const override;
    void update(const void* buf, size_t len) override;
    size_t finish(void* buf, size_t len) override;

private:
    SHA1_CTX _ctx;
};

}  // namespace rtcbase

#endif  //__RTCBASE_SHA1_DIGEST_H_


