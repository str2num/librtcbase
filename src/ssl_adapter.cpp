/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ssl_adapter.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "ssl_adapter.h"
#include "openssl_adapter.h"

namespace rtcbase {

bool initialize_SSL(VerificationCallback callback) {
    return OpenSSLAdapter::initialize_SSL(callback);
}

bool initialize_SSL_thread() {
    return OpenSSLAdapter::initialize_SSL_thread();
}

bool cleanup_SSL() {
    return OpenSSLAdapter::cleanup_SSL();
}

} // namespace rtcbase


