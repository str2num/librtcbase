/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file string_utils.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "string_utils.h"

namespace rtcbase {

static const char k_whitespace[] = " \n\r\t";

std::string string_trim(const std::string& s) {
    std::string::size_type first = s.find_first_not_of(k_whitespace);
    std::string::size_type last  = s.find_last_not_of(k_whitespace);

    if (first == std::string::npos || last == std::string::npos) {
        return std::string("");
    }

    return s.substr(first, last - first + 1);
}

} // namespace rtcbase


