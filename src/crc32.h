/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
 
/**
 * @file crc32.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_CRC32_H_
#define  __RTCBASE_CRC32_H_

#include <string>

#include "basic_types.h"

namespace rtcbase {

// Updates a CRC32 checksum with |len| bytes from |buf|. |initial| holds the
// checksum result from the previous update; for the first call, it should be 0.
uint32_t update_crc32(uint32_t initial, const void* buf, size_t len);

// Computes a CRC32 checksum using |len| bytes from |buf|.
inline uint32_t compute_crc32(const void* buf, size_t len) {
    return update_crc32(0, buf, len);
}

inline uint32_t compute_crc32(const std::string& str) {
    return compute_crc32(str.c_str(), str.size());
}

}  // namespace rtcbase

#endif  //__RTCBASE_CRC32_H_


