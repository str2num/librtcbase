/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file md5_digest.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_MD5_DIGEST_H_
#define  __RTCBASE_MD5_DIGEST_H_

#include "md5.h"
#include "message_digest.h"

namespace rtcbase {

// A simple wrapper for our MD5 implementation.
class Md5Digest : public MessageDigest {
public:
    enum { k_size = 16 };
    Md5Digest() {
        MD5_init(&_ctx);
    }
    size_t size() const override;
    void update(const void* buf, size_t len) override;
    size_t finish(void* buf, size_t len) override;

private:
    MD5Context _ctx;
};

}  // namespace rtcbase

#endif  //__RTCBASE_MD5_DIGEST_H_


