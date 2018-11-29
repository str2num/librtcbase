/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file message_digest.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_MESSAGE_DIGEST_H_
#define  __RTCBASE_MESSAGE_DIGEST_H_

#include <string>

#include "memcheck.h"

namespace rtcbase {

// Definitions for the digest algorithms.
extern const char DIGEST_MD5[];
extern const char DIGEST_SHA_1[];
extern const char DIGEST_SHA_224[];
extern const char DIGEST_SHA_256[];
extern const char DIGEST_SHA_384[];
extern const char DIGEST_SHA_512[];

// A general class for computing hashes.
class MessageDigest : public MemCheck {
public:
    enum { k_max_size = 64 };  // Maximum known size (SHA-512)
    MessageDigest() : MemCheck("MessageDigest") { }
    virtual ~MessageDigest() {}
    // Returns the digest output size (e.g. 16 bytes for MD5).
    virtual size_t size() const = 0;
    // Updates the digest with |len| bytes from |buf|.
    virtual void update(const void* buf, size_t len) = 0;
    // Outputs the digest value to |buf| with length |len|.
    // Returns the number of bytes written, i.e., Size().
    virtual size_t finish(void* buf, size_t len) = 0;
};

// A factory class for creating digest objects.
class MessageDigestFactory {
public:
    static MessageDigest* create(const std::string& alg);
};

// A whitelist of approved digest algorithms from RFC 4572 (FIPS 180).
bool is_fips180_digest_algorithm(const std::string& alg);

// Functions to create hashes.

// Computes the hash of |in_len| bytes of |input|, using the |digest| hash
// implementation, and outputs the hash to the buffer |output|, which is
// |out_len| bytes long. Returns the number of bytes written to |output| if
// successful, or 0 if |out_len| was too small.
size_t compute_digest(MessageDigest* digest, const void* input, size_t in_len,
        void* output, size_t out_len);

// Functions to compute RFC 2104 HMACs.

// Computes the HMAC of |in_len| bytes of |input|, using the |digest| hash
// implementation and |key_len| bytes of |key| to key the HMAC, and outputs
// the HMAC to the buffer |output|, which is |out_len| bytes long. Returns the
// number of bytes written to |output| if successful, or 0 if |out_len| was too
// small.
size_t compute_hmac(MessageDigest* digest, const void* key, size_t key_len,
        const void* input, size_t in_len,
        void* output, size_t out_len);

// Like the previous function, but creates a digest implementation based on
// the desired digest name |alg|, e.g. DIGEST_SHA_1. Returns 0 if there is no
// digest with the given name.
size_t compute_hmac(const std::string& alg, const void* key, size_t key_len,
        const void* input, size_t in_len,
        void* output, size_t out_len);


} // namespace rtcbase

#endif  //__RTCBASE_MESSAGE_DIGEST_H_


