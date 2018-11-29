/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file string_to_number.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <cerrno>
#include <cstdlib>

#include "string_to_number.h"

namespace rtcbase {

namespace string_to_number_internal {

rtcbase::Optional<signed_type> parse_signed(const char* str, int base) {
    if (!str) {
        return rtcbase::nullopt;
    }

    if (isdigit(str[0]) || str[0] == '-') {
        char* end = nullptr;
        errno = 0;
        const signed_type value = std::strtoll(str, &end, base);
        if (end && *end == '\0' && errno == 0) {
            return value;
        }
    }
    return rtcbase::nullopt;
}

rtcbase::Optional<unsigned_type> parse_unsigned(const char* str, int base) {
    if (!str) {
        return nullopt;
    }

    if (isdigit(str[0]) || str[0] == '-') {
        // Explicitly discard negative values. std::strtoull parsing causes unsigned
        // wraparound. We cannot just reject values that start with -, though, since
        // -0 is perfectly fine, as is -0000000000000000000000000000000.
        const bool is_negative = str[0] == '-';
        char* end = nullptr;
        errno = 0;
        const unsigned_type value = std::strtoull(str, &end, base);
        if (end && *end == '\0' && errno == 0 && (value == 0 || !is_negative)) {
            return value;
        }
    }
    return rtcbase::nullopt;
}

}  // namespace string_to_number_internal

}  // namespace rtcbase


