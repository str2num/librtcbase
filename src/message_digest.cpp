/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file message_digest.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <memory>

#include <string.h>

#include "basic_types.h"
#if SSL_USE_OPENSSL
#include "openssl_digest.h"
#else
#include "md5_digest.h"
#include "sha1_digest.h"
#endif
#include "string_encode.h"
#include "message_digest.h"

namespace rtcbase {

// From RFC 4572.
const char DIGEST_MD5[]     = "md5";
const char DIGEST_SHA_1[]   = "sha-1";
const char DIGEST_SHA_224[] = "sha-224";
const char DIGEST_SHA_256[] = "sha-256";
const char DIGEST_SHA_384[] = "sha-384";
const char DIGEST_SHA_512[] = "sha-512";

static const size_t k_block_size = 64;  // valid for SHA-256 and down

MessageDigest* MessageDigestFactory::create(const std::string& alg) {
#if SSL_USE_OPENSSL
    MessageDigest* digest = new OpenSSLDigest(alg);
    if (digest->size() == 0) {  // invalid algorithm
        delete digest;
        digest = NULL;
    }
    return digest;
#else
    MessageDigest* digest = NULL;
    if (alg == DIGEST_MD5) {
        digest = new Md5Digest();
    } else if (alg == DIGEST_SHA_1) {
        digest = new Sha1Digest();
    }
    return digest;
#endif
}

bool is_fips180_digest_algorithm(const std::string& alg) {
    // These are the FIPS 180 algorithms.  According to RFC 4572 Section 5,
    // "Self-signed certificates (for which legacy certificates are not a
    // consideration) MUST use one of the FIPS 180 algorithms (SHA-1,
    // SHA-224, SHA-256, SHA-384, or SHA-512) as their signature algorithm,
    // and thus also MUST use it to calculate certificate fingerprints."
    return alg == DIGEST_SHA_1 ||
        alg == DIGEST_SHA_224 ||
        alg == DIGEST_SHA_256 ||
        alg == DIGEST_SHA_384 ||
        alg == DIGEST_SHA_512;
}

size_t compute_digest(MessageDigest* digest, const void* input, size_t in_len,
        void* output, size_t out_len) 
{
    digest->update(input, in_len);
    return digest->finish(output, out_len);
}

// Compute a RFC 2104 HMAC: H(K XOR opad, H(K XOR ipad, text))
size_t compute_hmac(MessageDigest* digest,
        const void* key, size_t key_len,
        const void* input, size_t in_len,
        void* output, size_t out_len) 
{
    // We only handle algorithms with a 64-byte blocksize.
    // TODO: Add BlockSize() method to MessageDigest.
    size_t block_len = k_block_size;
    if (digest->size() > 32) {
        return 0;
    }
    // Copy the key to a block-sized buffer to simplify padding.
    // If the key is longer than a block, hash it and use the result instead.
    std::unique_ptr<uint8_t[]> new_key(new uint8_t[block_len]);
    if (key_len > block_len) {
        compute_digest(digest, key, key_len, new_key.get(), block_len);
        memset(new_key.get() + digest->size(), 0, block_len - digest->size());
    } else {
        memcpy(new_key.get(), key, key_len);
        memset(new_key.get() + key_len, 0, block_len - key_len);
    }
    // Set up the padding from the key, salting appropriately for each padding.
    std::unique_ptr<uint8_t[]> o_pad(new uint8_t[block_len]);
    std::unique_ptr<uint8_t[]> i_pad(new uint8_t[block_len]);
    for (size_t i = 0; i < block_len; ++i) {
        o_pad[i] = 0x5c ^ new_key[i];
        i_pad[i] = 0x36 ^ new_key[i];
    }
    // Inner hash; hash the inner padding, and then the input buffer.
    std::unique_ptr<uint8_t[]> inner(new uint8_t[digest->size()]);
    digest->update(i_pad.get(), block_len);
    digest->update(input, in_len);
    digest->finish(inner.get(), digest->size());
    // Outer hash; hash the outer padding, and then the result of the inner hash.
    digest->update(o_pad.get(), block_len);
    digest->update(inner.get(), digest->size());
    return digest->finish(output, out_len);
}

size_t compute_hmac(const std::string& alg, const void* key, size_t key_len,
        const void* input, size_t in_len,
        void* output, size_t out_len) 
{
    std::unique_ptr<MessageDigest> digest(MessageDigestFactory::create(alg));
    if (!digest) {
        return 0;
    }
    return compute_hmac(digest.get(), key, key_len,
            input, in_len, output, out_len);
}

} // namespace rtcbase


