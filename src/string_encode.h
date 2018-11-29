/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file string_encode.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_STRING_ENCODE_H_
#define  __RTCBASE_STRING_ENCODE_H_

#include <sstream>
#include <string>
#include <vector>

namespace rtcbase {

// Convert an unsigned value from 0 to 15 to the hex character equivalent...
char hex_encode(unsigned char val);
bool hex_decode(char ch, unsigned char* val);

// hex_encode shows the hex representation of binary data in ascii.
size_t hex_encode(char* buffer, size_t buflen,
        const char* source, size_t srclen);

// hex_encode, but separate each byte representation with a delimiter.
// |delimiter| == 0 means no delimiter
// If the buffer is too short, we return 0
size_t hex_encode_with_delimiter(char* buffer, size_t buflen,
        const char* source, size_t srclen,
        char delimiter);

// Helper functions for hex_encode.
std::string hex_encode(const std::string& str);
std::string hex_encode(const char* source, size_t srclen);
std::string hex_encode_with_delimiter(const char* source, size_t srclen,
        char delimiter);

// hex_decode, assuming that there is a delimiter between every byte
// pair.
// |delimiter| == 0 means no delimiter
// If the buffer is too short or the data is invalid, we return 0.
size_t hex_decode_with_delimiter(char* buffer, size_t buflen,
        const char* source, size_t srclen,
        char delimiter);

// Splits the source string into multiple fields separated by delimiter,
// with duplicates of delimiter creating empty fields.
size_t split(const std::string& source, char delimiter,
        std::vector<std::string>* fields);

// Splits the source string into multiple fields separated by delimiter,
// with duplicates of delimiter ignored.  Trailing delimiter ignored.
size_t tokenize(const std::string& source, char delimiter,
        std::vector<std::string>* fields);

// Extract the first token from source as separated by delimiter, with
// duplicates of delimiter ignored. Return false if the delimiter could not be
// found, otherwise return true.
bool tokenize_first(const std::string& source,
                    const char delimiter,
                    std::string* token,
                    std::string* rest);

template <class T>
static bool to_string(const T &t, std::string* s) {
    if (!s) {
        return false;
    }
    std::ostringstream oss;
    oss << std::boolalpha << t;
    *s = oss.str();
    return !oss.fail();
}

template<typename T>
static inline std::string to_string(const T& val) {
    std::string str; 
    to_string(val, &str); 
    return str;
}

template <class T>
static bool from_string(const std::string& s, T* t) {
    if (!t) {
        return false;
    }
    std::istringstream iss(s);
    iss >> std::boolalpha >> *t;
    return !iss.fail();
}

template<typename T>
static inline T from_string(const std::string& str) {
    T val; 
    from_string(str, &val); 
    return val;
}

template<typename T>
static inline T from_string(const T& default_value, const std::string& str) {
    T val(default_value); 
    from_string(str, &val); 
    return val;
}

} // namespace rtcbase

#endif  //__RTCBASE_STRING_ENCODE_H_


