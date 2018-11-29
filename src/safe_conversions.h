/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file safe_conversions.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SAFE_CONVERSIONS_H_
#define  __RTCBASE_SAFE_CONVERSIONS_H_

#include <limits>

#include "safe_conversions_impl.h"

namespace rtcbase {

// Convenience function that returns true if the supplied value is in range
// for the destination type.
template <typename Dst, typename Src>
inline bool is_value_in_range_for_numeric_type(Src value) {
    return internal::range_check<Dst>(value) == internal::TYPE_VALID;
}

// checked_cast<> and dchecked_cast<> are analogous to static_cast<> for
// numeric types, except that they [D]CHECK that the specified numeric
// conversion will not overflow or underflow. NaN source will always trigger
// the [D]CHECK.
template <typename Dst, typename Src>
inline Dst checked_cast(Src value) {
    //RTC_CHECK(IsValueInRangeForNumericType<Dst>(value));
    return static_cast<Dst>(value);
}

template <typename Dst, typename Src>
inline Dst dchecked_cast(Src value) {
    //RTC_DCHECK(IsValueInRangeForNumericType<Dst>(value));
    return static_cast<Dst>(value);
}

// saturated_cast<> is analogous to static_cast<> for numeric types, except
// that the specified numeric conversion will saturate rather than overflow or
// underflow. NaN assignment to an integral will trigger a RTC_CHECK condition.
template <typename Dst, typename Src>
inline Dst saturated_cast(Src value) {
    // Optimization for floating point values, which already saturate.
    if (std::numeric_limits<Dst>::is_iec559) {
        return static_cast<Dst>(value);
    }

    switch (internal::range_check<Dst>(value)) {
        case internal::TYPE_VALID:
            return static_cast<Dst>(value);

        case internal::TYPE_UNDERFLOW:
            return std::numeric_limits<Dst>::min();

        case internal::TYPE_OVERFLOW:
            return std::numeric_limits<Dst>::max();

            // Should fail only on attempting to assign NaN to a saturated integer.
        case internal::TYPE_INVALID:
            //FATAL();
            return std::numeric_limits<Dst>::max();
    }

    //FATAL();
    return static_cast<Dst>(value);
}

}  // namespace rtcbase

#endif  //__RTCBASE_SAFE_CONVERSIONS_H_


