/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file openssl_adapter.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_OPENSSL_ADAPTER_H_
#define  __RTCBASE_OPENSSL_ADAPTER_H_

#include <string>

#include "ssl_adapter.h"

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct x509_store_ctx_st X509_STORE_CTX;

namespace rtcbase {

class OpenSSLAdapter {
public:
    static bool initialize_SSL(VerificationCallback callback);
    static bool initialize_SSL_thread();
    static bool cleanup_SSL();

private:
    static void SSL_info_callback(const SSL* s, int where, int ret);
    static VerificationCallback _custom_verify_callback;
    friend class OpenSSLStreamAdapter;  // for _custom_verify_callback;
};

} // namespace rtcbase

#endif  //__RTCBASE_OPENSSL_ADAPTER_H_


