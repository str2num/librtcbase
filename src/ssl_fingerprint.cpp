/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ssl_fingerprint.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <string.h>
#include <algorithm>

#include "logging.h"
#include "message_digest.h"
#include "string_encode.h"
#include "ssl_fingerprint.h"

namespace rtcbase {

SSLFingerprint* SSLFingerprint::create(
        const std::string& algorithm, 
        const SSLIdentity* identity) 
{
    if (!identity) {
        return NULL;
    }

    return create(algorithm, &(identity->certificate()));
}

SSLFingerprint* SSLFingerprint::create(
        const std::string& algorithm, 
        const SSLCertificate* cert) 
{
    uint8_t digest_val[64];
    size_t digest_len;
    bool ret = cert->compute_digest(
            algorithm, digest_val, sizeof(digest_val), &digest_len);
    if (!ret) {
        return NULL;
    }

    return new SSLFingerprint(algorithm, digest_val, digest_len);
}

SSLFingerprint* SSLFingerprint::create_from_rfc4572(const std::string& algorithm,
        const std::string& fingerprint)
{
    if (algorithm.empty() || !rtcbase::is_fips180_digest_algorithm(algorithm)) {
        return NULL;
    }

    if (fingerprint.empty())
    return NULL;

    size_t value_len;
    char value[MessageDigest::k_max_size];
    value_len = hex_decode_with_delimiter(value, sizeof(value),
            fingerprint.c_str(),
            fingerprint.length(),
            ':');
    if (!value_len) {
        return NULL;
    }

    return new SSLFingerprint(algorithm, reinterpret_cast<uint8_t*>(value),
            value_len);    
}

SSLFingerprint::SSLFingerprint(const SSLFingerprint& from)
    : algorithm(from.algorithm) 
{
    this->digest_in = new uint8_t[from.digest_len];
    memcpy(this->digest_in, from.digest_in, from.digest_len);
    this->digest_len = from.digest_len;
}

SSLFingerprint::SSLFingerprint(const std::string& algorithm,
        const uint8_t* digest_in,
        size_t digest_len)
    : algorithm(algorithm) 
{
    this->digest_in = new uint8_t[digest_len];
    memcpy(this->digest_in, digest_in, digest_len);
    this->digest_len = digest_len;
}

SSLFingerprint::~SSLFingerprint() {
    if (digest_in) {
        delete[] digest_in;
        digest_in = NULL;
    }
}

bool SSLFingerprint::operator==(const SSLFingerprint& other) const {
    return algorithm == other.algorithm &&
        digest_len == other.digest_len &&
        0 == memcmp(digest_in, other.digest_in, digest_len);
}

std::string SSLFingerprint::get_rfc4572_fingerprint() const {
    std::string fingerprint =
        rtcbase::hex_encode_with_delimiter((const char*)digest_in, digest_len, ':');
    std::transform(fingerprint.begin(), fingerprint.end(),
            fingerprint.begin(), ::toupper);
    return fingerprint;
}

std::string SSLFingerprint::to_string() const {
    std::string fp_str = algorithm;
    fp_str.append(" ");
    fp_str.append(get_rfc4572_fingerprint());
    return fp_str;
}

} // namespace rtcbase


