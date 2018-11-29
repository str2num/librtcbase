/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file rate_statistics.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_RATE_STATISTICS_H_
#define  __RTCBASE_RATE_STATISTICS_H_

#include <memory>

#include "optional.h"

namespace rtcbase {

class RateStatistics {
public:
    static constexpr float k_bps_scale = 8000.0f;

    // max_window_size_ms = Maximum window size in ms for the rate estimation.
    //                      Initial window size is set to this, but may be changed
    //                      to something lower by calling set_window_size().
    // scale = coefficient to convert counts/ms to desired unit
    //         ex: k_bps_scale (8000) for bits/s if count represents bytes.
    RateStatistics(int64_t max_window_size_ms, float scale);
    ~RateStatistics();
    
    // Reset instance to original state.
    void reset();

    // Update rate with a new data point, moving averaging window as needed.
    void update(size_t count, int64_t now_ms);
    
    // Note that despite this being a const method, it still updates the internal
    // state (moves averaging window), but it doesn't make any alterations that
    // are observable from the other methods, as long as supplied timestamps are
    // from a monotonic clock. Ie, it doesn't matter if this call moves the
    // window, since any subsequent call to Update or Rate would still have moved
    // the window as much or more.
    rtcbase::Optional<uint32_t> rate(int64_t now_ms) const;
    
    // Update the size of the averaging window. The maximum allowed value for
    // window_size_ms is max_window_size_ms as supplied in the constructor.
    bool set_window_size(int64_t window_size_ms, int64_t now_ms);

private:
    void erase_old(int64_t now_ms);
    bool is_initialized() const;

private:
    // Counters are kept in buckets (circular buffer), with one bucket
    // per millisecond.
    struct Bucket {
        size_t sum;      // Sum of all samples in this bucket.
        size_t samples;  // Number of samples in this bucket.
    };
    std::unique_ptr<Bucket[]> _buckets;

    // Total count recorded in buckets.
    size_t _accumulated_count;

    // The total number of samples in the buckets.
    size_t _num_samples;

    // Oldest time recorded in buckets.
    int64_t _oldest_time;

    // Bucket index of oldest counter recorded in buckets.
    uint32_t _oldest_index;

    // To convert counts/ms to desired units
    const float _scale;

    // The window sizes, in ms, over which the rate is calculated.
    const int64_t _max_window_size_ms;
    int64_t _current_window_size_ms; 
};

} // namespace rtcbase

#endif  //__RTCBASE_RATE_STATISTICS_H_


