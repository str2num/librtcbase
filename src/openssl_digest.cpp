/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file openssl_digest.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "openssl_digest.h"

namespace rtcbase {

OpenSSLDigest::OpenSSLDigest(const std::string& algorithm) {
    EVP_MD_CTX_init(&_ctx);
    if (get_digest_EVP(algorithm, &_md)) {
        EVP_DigestInit_ex(&_ctx, _md, NULL);
    } else {
        _md = NULL;
    }
}

OpenSSLDigest::~OpenSSLDigest() {
    EVP_MD_CTX_cleanup(&_ctx);
}

bool OpenSSLDigest::get_digest_EVP(const std::string& algorithm,
        const EVP_MD** mdp) 
{
    const EVP_MD* md;
    if (algorithm == DIGEST_MD5) {
        md = EVP_md5();
    } else if (algorithm == DIGEST_SHA_1) {
        md = EVP_sha1();
    } else if (algorithm == DIGEST_SHA_224) {
        md = EVP_sha224();
    } else if (algorithm == DIGEST_SHA_256) {
        md = EVP_sha256();
    } else if (algorithm == DIGEST_SHA_384) {
        md = EVP_sha384();
    } else if (algorithm == DIGEST_SHA_512) {
        md = EVP_sha512();
    } else {
        return false;
    }

    // Can't happen
    //ASSERT(EVP_MD_size(md) >= 16);
    *mdp = md;
    return true;
}

bool OpenSSLDigest::get_digest_size(const std::string& algorithm,
        size_t* length) 
{
    const EVP_MD *md;
    if (!get_digest_EVP(algorithm, &md)) {
        return false;
    }

    *length = EVP_MD_size(md);
    return true;
}

} // namespace webrtc


