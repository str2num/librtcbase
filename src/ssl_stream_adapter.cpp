/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 

/**
 * @file ssl_stream_adapter.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "openssl_stream_adapter.h"
#include "ssl_stream_adapter.h"

namespace rtcbase {

std::string srtp_crypto_suite_to_name(int crypto_suite) {
    switch (crypto_suite) {
        case SRTP_AES128_CM_SHA1_32:
            return CS_AES_CM_128_HMAC_SHA1_32;
        case SRTP_AES128_CM_SHA1_80:
            return CS_AES_CM_128_HMAC_SHA1_80;
        case SRTP_AEAD_AES_128_GCM:
            return CS_AEAD_AES_128_GCM;
        case SRTP_AEAD_AES_256_GCM:
            return CS_AEAD_AES_256_GCM;
        default:
            return std::string();
    }
}

int srtp_crypto_suite_from_name(const std::string& crypto_suite) {
    if (crypto_suite == CS_AES_CM_128_HMAC_SHA1_32) {
        return SRTP_AES128_CM_SHA1_32;
    }
    if (crypto_suite == CS_AES_CM_128_HMAC_SHA1_80) {
        return SRTP_AES128_CM_SHA1_80;
    }
    if (crypto_suite == CS_AEAD_AES_128_GCM) {
        return SRTP_AEAD_AES_128_GCM;
    }
    if (crypto_suite == CS_AEAD_AES_256_GCM) {
        return SRTP_AEAD_AES_256_GCM;
    }
    return SRTP_INVALID_CRYPTO_SUITE;
}

bool get_srtp_key_and_salt_lengths(int crypto_suite, int* key_length,
        int* salt_length) 
{
    switch (crypto_suite) {
        case SRTP_AES128_CM_SHA1_32:
        case SRTP_AES128_CM_SHA1_80:
            // SRTP_AES128_CM_HMAC_SHA1_32 and SRTP_AES128_CM_HMAC_SHA1_80 are defined
            // in RFC 5764 to use a 128 bits key and 112 bits salt for the cipher.
            *key_length = 16;
            *salt_length = 14;
            break;
        case SRTP_AEAD_AES_128_GCM:
            // SRTP_AEAD_AES_128_GCM is defined in RFC 7714 to use a 128 bits key and
            // a 96 bits salt for the cipher.
            *key_length = 16;
            *salt_length = 12;
            break;
        case SRTP_AEAD_AES_256_GCM:
            // SRTP_AEAD_AES_256_GCM is defined in RFC 7714 to use a 256 bits key and
            // a 96 bits salt for the cipher.
            *key_length = 32;
            *salt_length = 12;
            break;
        default:
            return false;
    }
    return true;
}

bool is_gcm_crypto_suite_name(const std::string& crypto_suite) {
    return (crypto_suite == CS_AEAD_AES_256_GCM ||
            crypto_suite == CS_AEAD_AES_128_GCM);
}

// static
CryptoOptions CryptoOptions::no_gcm() {
    CryptoOptions options;
    options.enable_gcm_crypto_suites = false;
    return options;
}

std::vector<int> get_supported_dtls_srtp_crypto_suites(
        const CryptoOptions& crypto_options) 
{
    std::vector<int> crypto_suites;
    if (crypto_options.enable_gcm_crypto_suites) {
        crypto_suites.push_back(SRTP_AEAD_AES_256_GCM);
        crypto_suites.push_back(SRTP_AEAD_AES_128_GCM);
    }
    // Note: SRTP_AES128_CM_SHA1_80 is what is required to be supported (by
    // draft-ietf-rtcweb-security-arch), but SRTP_AES128_CM_SHA1_32 is allowed as
    // well, and saves a few bytes per packet if it ends up selected.
    crypto_suites.push_back(SRTP_AES128_CM_SHA1_32);
    crypto_suites.push_back(SRTP_AES128_CM_SHA1_80);
    return crypto_suites;
}

SSLStreamAdapter* SSLStreamAdapter::create(StreamInterface* stream, EventLoop* el) {
    return new OpenSSLStreamAdapter(stream, el);
}

bool SSLStreamAdapter::get_ssl_cipher_suite(int* cipher_suite) {
    (void)cipher_suite;
    return false;
}

bool SSLStreamAdapter::export_keying_material(const std::string& label,
        const uint8_t* context,
        size_t context_len,
        bool use_context,
        uint8_t* result,
        size_t result_len) 
{
    (void)label;
    (void)context;
    (void)context_len;
    (void)use_context;
    (void)result;
    (void)result_len;
    return false;  // Default is unsupported
}

bool SSLStreamAdapter::set_dtls_srtp_crypto_suites(
        const std::vector<int>& crypto_suites) 
{
    (void)crypto_suites;
    return false;
}

bool SSLStreamAdapter::get_dtls_srtp_crypto_suite(int* crypto_suite) {
    (void)crypto_suite;
    return false;
}

} // namespace rtcbase


