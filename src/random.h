/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file random.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_RANDOM_H_
#define  __RTCBASE_RANDOM_H_

#include <limits>
#include <string>

#include "constructor_magic.h"
#include "basic_types.h"

namespace rtcbase {

class Random {
public:
    // TODO(tommi): Change this so that the seed can be initialized internally,
    // e.g. by offering two ways of constructing or offer a static method that
    // returns a seed that's suitable for initialization.
    // The problem now is that callers are calling clock_->TimeInMicroseconds()
    // which calls TickTime::Now().Ticks(), which can return a very low value on
    // Mac and can result in a seed of 0 after conversion to microseconds.
    // Besides the quality of the random seed being poor, this also requires
    // the client to take on extra dependencies to generate a seed.
    // If we go for a static seed generator in Random, we can use something from
    // webrtc/rtc_base and make sure that it works the same way across platforms.
    // See also discussion here: https://codereview.webrtc.org/1623543002/
    explicit Random(uint64_t seed);

    // Return pseudo-random integer of the specified type.
    // We need to limit the size to 32 bits to keep the output close to uniform.
    template <typename T>
    T rand() {
        static_assert(std::numeric_limits<T>::is_integer &&
                std::numeric_limits<T>::radix == 2 &&
                std::numeric_limits<T>::digits <= 32,
                "Rand is only supported for built-in integer types that are "
                "32 bits or smaller.");
        return static_cast<T>(next_output());
    }

    // Uniformly distributed pseudo-random number in the interval [0, t].
    uint32_t rand(uint32_t t);

    // Uniformly distributed pseudo-random number in the interval [low, high].
    uint32_t rand(uint32_t low, uint32_t high);

    // Uniformly distributed pseudo-random number in the interval [low, high].
    int32_t rand(int32_t low, int32_t high);

    // Normal Distribution.
    double gaussian(double mean, double standard_deviation);

    // Exponential Distribution.
    double exponential(double lambda);

private:
    // Outputs a nonzero 64-bit random number.
    uint64_t next_output() {
        _state ^= _state >> 12;
        _state ^= _state << 25;
        _state ^= _state >> 27;
        return _state * 2685821657736338717ull;
    }

    uint64_t _state;

    RTC_DISALLOW_IMPLICIT_CONSTRUCTORS(Random);
};

// Return pseudo-random number in the interval [0.0, 1.0).
template <>
float Random::rand<float>();

// Return pseudo-random number in the interval [0.0, 1.0).
template <>
double Random::rand<double>();

// Return pseudo-random boolean value.
template <>
bool Random::rand<bool>();

// Generates a (cryptographically) random string of the given length.
// We generate base64 values so that they will be printable.
std::string create_random_string(size_t length);

// Generates a (cryptographically) random string of the given length.
// We generate base64 values so that they will be printable.
// Return false if the random number generator failed.
bool create_random_string(size_t length, std::string* str);

// Generates (cryptographically) random data of the given length.
// Return false if the random number generator failed.
bool create_random_data(size_t length, std::string* data);

// Generates a (cryptographically) random UUID version 4 string.
std::string create_random_uuid();

// Generates a random id.
uint32_t create_random_id();

// Generates a 64 bit random id.
uint64_t create_random_id64();

// Generates a random id > 0.
uint32_t create_random_non_zero_id();

} // namespace rtcbase

#endif  //__RTCBASE_RANDOM_H_


