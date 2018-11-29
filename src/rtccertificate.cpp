/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file rtccertificate.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <memory>

#include "time_utils.h"
#include "rtccertificate.h"

namespace rtcbase {

RTCCertificate* RTCCertificate::create(std::unique_ptr<SSLIdentity> identity) {
    return new RTCCertificate(identity.release());
}

RTCCertificate::RTCCertificate(SSLIdentity* identity) 
    : _identity(identity) 
{
}

RTCCertificate::~RTCCertificate() {

}

uint64_t RTCCertificate::expires() const {
    int64_t expires = ssl_certificate().certificate_expiration_time();
    if (expires != -1) {
        return static_cast<uint64_t>(expires) * k_num_millisecs_per_sec;
    }
    // If the expiration time could not be retrieved return an expired timestamp.
    return 0;  // = 1970-01-01
}

bool RTCCertificate::has_expired(uint64_t now) const {
    return expires() <= now;
}

const SSLCertificate& RTCCertificate::ssl_certificate() const {
    return _identity->certificate();
}

} // namespace rtcbase


