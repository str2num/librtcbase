/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ssl_adapter.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SSL_ADAPTER_H_
#define  __RTCBASE_SSL_ADAPTER_H_

#include "ssl_stream_adapter.h"

namespace rtcbase {

typedef bool (*VerificationCallback)(void* cert);

// Call this on the main thread, before using SSL.
// Call CleanupSSLThread when finished with SSL.
bool initialize_SSL(VerificationCallback callback = NULL);

// Call to initialize additional threads.
bool initialize_SSL_thread();

// Call to cleanup additional threads, and also the main thread.
bool cleanup_SSL();

} // namespace rtcbase

#endif  //__RTCBASE_SSL_ADAPTER_H_


