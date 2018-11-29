/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file openssl_digest.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_OPENSSL_DIGEST_H_
#define  __RTCBASE_OPENSSL_DIGEST_H_

#include <openssl/evp.h>

#include "message_digest.h"

namespace rtcbase {

class OpenSSLDigest : MessageDigest {
public:
    // Creates an OpenSSLDigest with |algorithm| as the hash algorithm.
    explicit OpenSSLDigest(const std::string& algorithm);
    ~OpenSSLDigest() override;

    // Helper function to look up a digest's EVP by name.
    static bool get_digest_EVP(const std::string &algorithm,
            const EVP_MD** md);
    
    // Helper function to get the length of a digest.
    static bool get_digest_size(const std::string &algorithm,
            size_t* len);

private:
    EVP_MD_CTX _ctx;
    const EVP_MD* _md;
};

} // namespace rtcbase

#endif  //__RTCBASE_OPENSSL_DIGEST_H_


