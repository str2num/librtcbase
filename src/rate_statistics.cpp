/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file rate_statistics.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "rate_statistics.h"

namespace rtcbase {

RateStatistics::RateStatistics(int64_t window_size_ms, float scale)
    : _buckets(new Bucket[window_size_ms]()),
    _accumulated_count(0),
    _num_samples(0),
    _oldest_time(-window_size_ms),
    _oldest_index(0),
    _scale(scale),
    _max_window_size_ms(window_size_ms),
    _current_window_size_ms(_max_window_size_ms) {}

RateStatistics::~RateStatistics() {}

void RateStatistics::erase_old(int64_t now_ms) {
    if (!is_initialized()) {
        return;
    }

    // New oldest time that is included in data set.
    int64_t new_oldest_time = now_ms - _current_window_size_ms + 1;

    // New oldest time is older than the current one, no need to cull data.
    if (new_oldest_time <= _oldest_time) {
        return;
    }

    // Loop over buckets and remove too old data points.
    while (_num_samples > 0 && _oldest_time < new_oldest_time) {
        const Bucket& oldest_bucket = _buckets[_oldest_index];
        _accumulated_count -= oldest_bucket.sum;
        _num_samples -= oldest_bucket.samples;
        _buckets[_oldest_index] = Bucket();
        if (++_oldest_index >= _max_window_size_ms) {
            _oldest_index = 0;
        }
        ++_oldest_time;
    }
    _oldest_time = new_oldest_time;
}

void RateStatistics::reset() {
    _accumulated_count = 0;
    _num_samples = 0;
    _oldest_time = -_max_window_size_ms;
    _oldest_index = 0;
    _current_window_size_ms = _max_window_size_ms;
    for (int64_t i = 0; i < _max_window_size_ms; i++) {
        _buckets[i] = Bucket();
    }
}

void RateStatistics::update(size_t count, int64_t now_ms) {
    if (now_ms < _oldest_time) {
        // Too old data is ignored.
        return;
    }

    erase_old(now_ms);

    // First ever sample, reset window to start now.
    if (!is_initialized()) {
        _oldest_time = now_ms;
    }

    uint32_t now_offset = static_cast<uint32_t>(now_ms - _oldest_time);
    uint32_t index = _oldest_index + now_offset;
    if (index >= _max_window_size_ms) {
        index -= _max_window_size_ms;
    }
    _buckets[index].sum += count;
    ++_buckets[index].samples;
    _accumulated_count += count;
    ++_num_samples;
}

rtcbase::Optional<uint32_t> RateStatistics::rate(int64_t now_ms) const {
    // Yeah, this const_cast ain't pretty, but the alternative is to declare most
    // of the members as mutable...
    const_cast<RateStatistics*>(this)->erase_old(now_ms);

    // If window is a single bucket or there is only one sample in a data set that
    // has not grown to the full window size, treat this as rate unavailable.
    int64_t active_window_size = now_ms - _oldest_time + 1;
    if (_num_samples == 0 || active_window_size <= 1 ||
            (_num_samples <= 1 && active_window_size < _current_window_size_ms)) 
    {
        return rtcbase::nullopt;
    }

    float scale = _scale / active_window_size;
    return static_cast<uint32_t>(_accumulated_count * scale + 0.5f);
}

bool RateStatistics::set_window_size(int64_t window_size_ms, int64_t now_ms) {
    if (window_size_ms <= 0 || window_size_ms > _max_window_size_ms) {
        return false;
    }

    _current_window_size_ms = window_size_ms;
    erase_old(now_ms);
    return true;
}

bool RateStatistics::is_initialized() const {
    return _oldest_time != -_max_window_size_ms;
}

} // namespace rtcbase


