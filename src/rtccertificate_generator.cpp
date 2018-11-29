/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file rtccertificate_generator.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "rtccertificate_generator.h"

namespace rtcbase {

namespace {

const char k_identity_name[] = "WebRTC";
const uint64_t k_year_in_seconds = 365 * 24 * 60 * 60; // two years

}

RTCCertificate* RTCCertificateGenerator::generate_certificate(
        const KeyParams& key_params,
        uint64_t expires_ms)
{
    if (!key_params.is_valid()) {
        return nullptr;
    }

    SSLIdentity* identity;
    if (!expires_ms) {
        identity = SSLIdentity::generate(k_identity_name, key_params);
    } else {
        uint64_t expires_s = expires_ms / 1000;
        expires_s = std::min(expires_s, k_year_in_seconds);
        time_t cert_lifetime_s = static_cast<time_t>(expires_s);
        identity = SSLIdentity::generate_with_expiration(k_identity_name,
                key_params, cert_lifetime_s);
    }

    if (!identity) {
        return nullptr;
    }

    std::unique_ptr<SSLIdentity> identity_sptr(identity);
    return RTCCertificate::create(std::move(identity_sptr));
}

} // namespace rtcbase


