/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file random.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <limits>
#include <memory>
#include <math.h>

#include <openssl/rand.h>

#include "logging.h"
#include "basic_types.h"
#include "random.h"

namespace rtcbase {

Random::Random(uint64_t seed) {
    _state = seed;
}

uint32_t Random::rand(uint32_t t) {
    // Casting the output to 32 bits will give an almost uniform number.
    // Pr[x=0] = (2^32-1) / (2^64-1)
    // Pr[x=k] = 2^32 / (2^64-1) for k!=0
    // Uniform would be Pr[x=k] = 2^32 / 2^64 for all 32-bit integers k.
    uint32_t x = next_output();
    // If x / 2^32 is uniform on [0,1), then x / 2^32 * (t+1) is uniform on
    // the interval [0,t+1), so the integer part is uniform on [0,t].
    uint64_t result = x * (static_cast<uint64_t>(t) + 1);
    result >>= 32;
    return result;
}

uint32_t Random::rand(uint32_t low, uint32_t high) {
    return rand(high - low) + low;
}

int32_t Random::rand(int32_t low, int32_t high) {
    const int64_t low_i64{low};
    return static_cast<int32_t>(
            rand(static_cast<uint32_t>(high - low_i64)) + low_i64);
}

template <>
float Random::rand<float>() {
    double result = next_output() - 1;
    result = result / 0xFFFFFFFFFFFFFFFEull;
    return static_cast<float>(result);
}

template <>
double Random::rand<double>() {
    double result = next_output() - 1;
    result = result / 0xFFFFFFFFFFFFFFFEull;
    return result;
}

template <>
bool Random::rand<bool>() {
    return rand(0, 1) == 1;
}

double Random::gaussian(double mean, double standard_deviation) {
    // Creating a Normal distribution variable from two independent uniform
    // variables based on the Box-Muller transform, which is defined on the
    // interval (0, 1]. Note that we rely on NextOutput to generate integers
    // in the range [1, 2^64-1]. Normally this behavior is a bit frustrating,
    // but here it is exactly what we need.
    const double k_pi = 3.14159265358979323846;
    double u1 = static_cast<double>(next_output()) / 0xFFFFFFFFFFFFFFFFull;
    double u2 = static_cast<double>(next_output()) / 0xFFFFFFFFFFFFFFFFull;
    return mean + standard_deviation * sqrt(-2 * log(u1)) * cos(2 * k_pi * u2);
}

double Random::exponential(double lambda) {
    double uniform = rand<double>();
    return -log(uniform) / lambda;
}

// Base class for RNG implementations.
class RandomGenerator {
public:
    virtual ~RandomGenerator() {}
    virtual bool init(const void* seed, size_t len) = 0;
    virtual bool generate(void* buf, size_t len) = 0;
};

// The OpenSSL RNG.
class SecureRandomGenerator : public RandomGenerator {
public:
    SecureRandomGenerator() {}
    ~SecureRandomGenerator() override {}
    bool init(const void* seed, size_t len) override  { 
        (void)seed;
        (void)len; 
        return true; 
    }
    bool generate(void* buf, size_t len) override {
        return (RAND_bytes(reinterpret_cast<unsigned char*>(buf), len) > 0);
    }
};

namespace {

// TODO: Use Base64::Base64Table instead.
static const char k_base64[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static const char k_hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static const char k_uuid_digit17[4] = {'8', '9', 'a', 'b'};

// This round about way of creating a global RNG is to safe-guard against
// indeterminant static initialization order.
std::unique_ptr<RandomGenerator>& get_global_rng() {
    RTC_DEFINE_STATIC_LOCAL(std::unique_ptr<RandomGenerator>, global_rng,
            (new SecureRandomGenerator()));
    return global_rng;
}

RandomGenerator& rng() {
    return *get_global_rng();
}

}  // namespace

std::string create_random_string(size_t len) {
    std::string str;
    create_random_string(len, &str);
    return str;
}

static bool create_random_string(size_t len,
        const char* table, int table_size,
        std::string* str) 
{
    str->clear();
    // Avoid biased modulo division below.
    if (256 % table_size) {
        LOG(LS_FATAL) << "Table size must divide 256 evenly!";
        return false;
    }
    std::unique_ptr<uint8_t[]> bytes(new uint8_t[len]);
    if (!rng().generate(bytes.get(), len)) {
        LOG(LS_FATAL) << "Failed to generate random string!";
        return false;
    }
    str->reserve(len);
    for (size_t i = 0; i < len; ++i) {
        str->push_back(table[bytes[i] % table_size]);
    }
    return true;
}

bool create_random_string(size_t len, std::string* str) {
    return create_random_string(len, k_base64, 64, str);
}

bool create_random_data(size_t length, std::string* data) {
    data->resize(length);
    // std::string is guaranteed to use contiguous memory in c++11 so we can
    // safely write directly to it.
    return rng().generate(&data->at(0), length);
}

// Version 4 UUID is of the form:
// xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
// Where 'x' is a hex digit, and 'y' is 8, 9, a or b.
std::string create_random_uuid() {
    std::string str;
    std::unique_ptr<uint8_t[]> bytes(new uint8_t[31]);
    rng().generate(bytes.get(), 31);
    str.reserve(36);
    for (size_t i = 0; i < 8; ++i) {
        str.push_back(k_hex[bytes[i] % 16]);
    }
    str.push_back('-');
    for (size_t i = 8; i < 12; ++i) {
        str.push_back(k_hex[bytes[i] % 16]);
    }
    str.push_back('-');
    str.push_back('4');
    for (size_t i = 12; i < 15; ++i) {
        str.push_back(k_hex[bytes[i] % 16]);
    }
    str.push_back('-');
    str.push_back(k_uuid_digit17[bytes[15] % 4]);
    for (size_t i = 16; i < 19; ++i) {
        str.push_back(k_hex[bytes[i] % 16]);
    }
    str.push_back('-');
    for (size_t i = 19; i < 31; ++i) {
        str.push_back(k_hex[bytes[i] % 16]);
    }
    return str;
}

uint32_t create_random_id() {
    uint32_t id;
    rng().generate(&id, sizeof(id));
    return id;
}

uint64_t create_random_id64() {
    return static_cast<uint64_t>(create_random_id()) << 32 | create_random_id();
}

uint32_t create_random_non_zero_id() {
    uint32_t id;
    do {
        id = create_random_id();
    } while (id == 0);
    return id;
}

} // namespace rtcbase


