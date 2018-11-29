/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ssl_fingerprint.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SSL_FINGERPRINT_H_
#define  __RTCBASE_SSL_FINGERPRINT_H_

#include "ssl_identity.h"

namespace rtcbase {

struct SSLFingerprint {
    static SSLFingerprint* create(const std::string& algorithm,
            const SSLIdentity* identity);
    
    static SSLFingerprint* create(const std::string& algorithm,
            const SSLCertificate* cert);

    static SSLFingerprint* create_from_rfc4572(const std::string& algorithm,
            const std::string& fingerprint);

    SSLFingerprint(const std::string& algorithm,
            const uint8_t* digest_in,
            size_t digest_len);
    SSLFingerprint(const SSLFingerprint& from);
    ~SSLFingerprint();
    
    bool operator==(const SSLFingerprint& other) const;
    std::string get_rfc4572_fingerprint() const;
    std::string to_string() const;

    std::string algorithm;
    uint8_t* digest_in;
    size_t digest_len;
};

} // namespace rtcbase

#endif  //__SSL_FINGERPRINT_H_


