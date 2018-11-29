/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
/**
 * @file rtccertificate.h
 * @author str2num
 * @brief 
 *  
 **/

#ifndef  __RTCBASE_RTCCERTIFICATE_H_
#define  __RTCBASE_RTCCERTIFICATE_H_

#include <memory>

#include "ssl_identity.h"

namespace rtcbase {

class RTCCertificate {
public:
    static RTCCertificate* create(std::unique_ptr<SSLIdentity> identity);
    ~RTCCertificate();
    
    // Returns the expiration time in ms relative to epoch, 1970-01-01T00:00:00Z.
    uint64_t expires() const;
    // Checks if the certificate has expired, where |now| is expressed in ms
    // relative to epoch, 1970-01-01T00:00:00Z.
    bool has_expired(uint64_t now) const;

    const SSLCertificate& ssl_certificate() const;
    SSLIdentity* identity() const { return _identity.get(); }
    
protected:
    explicit RTCCertificate(SSLIdentity* identity);
    
private:
    std::unique_ptr<SSLIdentity> _identity;
};

} // namespace rtcbase

#endif  //__RTCBASE_RTCCERTIFICATE_H_


