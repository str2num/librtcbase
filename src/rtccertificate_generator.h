/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file rtccertificate_generator.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_RTCCERTIFICATE_GENERATOR_H_
#define  __RTCBASE_RTCCERTIFICATE_GENERATOR_H_

#include "rtccertificate.h"
#include "ssl_identity.h"

namespace rtcbase {

class RTCCertificateGenerator {
public:
    RTCCertificateGenerator() {}
    ~RTCCertificateGenerator() {}

    static RTCCertificate* generate_certificate(const KeyParams& key_params,
            uint64_t expires_ms);
};

} // namespace rtcbase

#endif  //__RTCBASE_RTCCERTIFICATE_GENERATOR_H_


