/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file md5.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_MD5_H_
#define  __RTCBASE_MD5_H_

#include <stdint.h>
#include <stdlib.h>

namespace rtcbase {

struct MD5Context {
    uint32_t buf[4];
    uint32_t bits[2];
    uint32_t in[16];
};

void MD5_init(MD5Context* context);
void MD5_update(MD5Context* context, const uint8_t* data, size_t len);
void MD5_final(MD5Context* context, uint8_t digest[16]);
void MD5_transform(uint32_t buf[4], const uint32_t in[16]);

}  // namespace rtcbase

#endif  //__RTCBASE_MD5_H_


