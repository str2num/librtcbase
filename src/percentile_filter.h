/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file percentile_filter.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_PERCENTILE_FILTER_H_
#define  __RTCBASE_PERCENTILE_FILTER_H_

#include <stdint.h>

#include <iterator>
#include <set>

namespace rtcbase {

// Class to efficiently get the percentile value from a group of observations.
// The percentile is the value below which a given percentage of the
// observations fall.
template <typename T>
class PercentileFilter {
public:
    // Construct filter. |percentile| should be between 0 and 1.
    explicit PercentileFilter(float percentile);

    // Insert one observation. The complexity of this operation is logarithmic in
    // the size of the container.
    void insert(const T& value);

    // Remove one observation or return false if |value| doesn't exist in the
    // container. The complexity of this operation is logarithmic in the size of
    // the container.
    bool erase(const T& value);

    // Get the percentile value. The complexity of this operation is constant.
    T get_percentile_value() const;

    // Removes all the stored observations.
    void reset();

private:
    // Update iterator and index to point at target percentile value.
    void update_percentile_iterator();

    const float _percentile;
    std::multiset<T> _set;
    // Maintain iterator and index of current target percentile value.
    typename std::multiset<T>::iterator _percentile_it;
    int64_t _percentile_index;
};

template <typename T>
PercentileFilter<T>::PercentileFilter(float percentile)
    : _percentile(percentile),
    _percentile_it(_set.begin()),
    _percentile_index(0) 
{
    //RTC_CHECK_GE(percentile, 0.0f);
    //RTC_CHECK_LE(percentile, 1.0f);
}

template <typename T>
void PercentileFilter<T>::insert(const T& value) {
    // Insert element at the upper bound.
    _set.insert(value);
    if (_set.size() == 1u) {
        // First element inserted - initialize percentile iterator and index.
        _percentile_it = _set.begin();
        _percentile_index = 0;
    } else if (value < *_percentile_it) {
        // If new element is before us, increment |percentile_index_|.
        ++_percentile_index;
    }
    update_percentile_iterator();
}

template <typename T>
bool PercentileFilter<T>::erase(const T& value) {
typename std::multiset<T>::const_iterator it = _set.lower_bound(value);
    // Ignore erase operation if the element is not present in the current set.
    if (it == _set.end() || *it != value) {
        return false;
    }
    if (it == _percentile_it) {
        // If same iterator, update to the following element. Index is not
        // affected.
        _percentile_it = _set.erase(it);
    } else {
        _set.erase(it);
        // If erased element was before us, decrement |percentile_index_|.
        if (value <= *_percentile_it) {
            --_percentile_index;
        }
    }
    update_percentile_iterator();
    return true;
}

template <typename T>
void PercentileFilter<T>::update_percentile_iterator() {
    if (_set.empty()) {
        return;
    }
    const int64_t index = static_cast<int64_t>(_percentile * (_set.size() - 1));
    std::advance(_percentile_it, index - _percentile_index);
    _percentile_index = index;
}

template <typename T>
T PercentileFilter<T>::get_percentile_value() const {
    return _set.empty() ? 0 : *_percentile_it;
}

template <typename T>
void PercentileFilter<T>::reset() {
    _set.clear();
    _percentile_it = _set.begin();
    _percentile_index = 0;
}

}  // namespace rtcbase

#endif  //__RTCBASE_PERCENTILE_FILTER_H_


