/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file moving_median_filter.h
 * @author str2num
 * @brief 
 *  
 **/

#ifndef  __MOVING_MEDIAN_FILTER_H_
#define  __MOVING_MEDIAN_FILTER_H_

#include <list>

#include "constructor_magic.h"
#include "percentile_filter.h"

namespace rtcbase {

// Class to efficiently get moving median filter from a stream of samples.
template <typename T>
class MovingMedianFilter {
public:
    // Construct filter. |window_size| is how many latest samples are stored and
    // used to take median. |window_size| must be positive.
    explicit MovingMedianFilter(size_t window_size);

    // Insert a new sample.
    void insert(const T& value);

    // Removes all samples;
    void reset();

    // Get median over the latest window.
    T get_filtered_value() const;

private:
    PercentileFilter<T> _percentile_filter;
    std::list<T> _samples;
    size_t _samples_stored;
    const size_t _window_size;

    RTC_DISALLOW_COPY_AND_ASSIGN(MovingMedianFilter);
};

template <typename T>
MovingMedianFilter<T>::MovingMedianFilter(size_t window_size)
    : _percentile_filter(0.5f), _samples_stored(0), _window_size(window_size) 
{
    //RTC_CHECK_GT(window_size, 0);
}

template <typename T>
void MovingMedianFilter<T>::insert(const T& value) {
    _percentile_filter.insert(value);
    _samples.emplace_back(value);
    ++_samples_stored;
    if (_samples_stored > _window_size) {
        _percentile_filter.erase(_samples.front());
        _samples.pop_front();
        --_samples_stored;
    }
}

template <typename T>
T MovingMedianFilter<T>::get_filtered_value() const {
    return _percentile_filter.get_percentile_value();
}

template <typename T>
void MovingMedianFilter<T>::reset() {
    _percentile_filter.reset();
    _samples.clear();
    _samples_stored = 0;
}

}  // namespace rtcbase

#endif  //__MOVING_MEDIAN_FILTER_H_


