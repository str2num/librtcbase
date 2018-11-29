/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
/**
 * @file openssl.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_OPENSSL_H_
#define  __RTCBASE_OPENSSL_H_

#include <openssl/ssl.h>

#if (OPENSSL_VERSION_NUMBER < 0x10000000L)
#error OpenSSL is older than 1.0.0, which is the minimum supported version.
#endif

#endif  //__RTCBASE_OPENSSL_H_


