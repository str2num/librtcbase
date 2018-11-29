/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file string_encode.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "string_utils.h"
#include "string_encode.h"

namespace rtcbase {

static const char HEX[] = "0123456789abcdef";

char hex_encode(unsigned char val) {
    return (val < 16) ? HEX[val] : '!';
}

bool hex_decode(char ch, unsigned char* val) {
    if ((ch >= '0') && (ch <= '9')) {
        *val = ch - '0';
    } else if ((ch >= 'A') && (ch <= 'Z')) {
        *val = (ch - 'A') + 10;
    } else if ((ch >= 'a') && (ch <= 'z')) {
        *val = (ch - 'a') + 10;
    } else {
        return false;
    }
    return true;
}

size_t hex_encode(char* buffer, size_t buflen,
        const char* csource, size_t srclen) 
{
    return hex_encode_with_delimiter(buffer, buflen, csource, srclen, 0);
}

size_t hex_encode_with_delimiter(char* buffer, size_t buflen,
        const char* csource, size_t srclen,
        char delimiter) 
{
    if (!buffer || buflen == 0) {  // TODO(grunell): estimate output size_t
        return 0;
    }

    // Init and check bounds.
    const unsigned char* bsource =
        reinterpret_cast<const unsigned char*>(csource);
    size_t srcpos = 0, bufpos = 0;
    size_t needed = delimiter ? (srclen * 3) : (srclen * 2 + 1);
    if (buflen < needed) {
        return 0;
    }

    while (srcpos < srclen) {
        unsigned char ch = bsource[srcpos++];
        buffer[bufpos  ] = hex_encode((ch >> 4) & 0xF);
        buffer[bufpos+1] = hex_encode((ch     ) & 0xF);
        bufpos += 2;

        // Don't write a delimiter after the last byte.
        if (delimiter && (srcpos < srclen)) {
            buffer[bufpos] = delimiter;
            ++bufpos;
        }
    }

    // Null terminate.
    buffer[bufpos] = '\0';
    return bufpos;
}

std::string hex_encode(const std::string& str) {
    return hex_encode(str.c_str(), str.size());
}

std::string hex_encode(const char* source, size_t srclen) {
    return hex_encode_with_delimiter(source, srclen, 0);
}

std::string hex_encode_with_delimiter(const char* source, size_t srclen,
        char delimiter) 
{
    const size_t k_buffer_size = srclen * 3;
    char* buffer = STACK_ARRAY(char, k_buffer_size);
    size_t length = hex_encode_with_delimiter(buffer, k_buffer_size,
            source, srclen, delimiter);
    if (srclen != 0 && length <= 0) {
        return "";
    }
    return std::string(buffer, length);
}

size_t hex_decode_with_delimiter(char* cbuffer, size_t buflen,
        const char* source, size_t srclen,
        char delimiter) 
{
    //RTC_DCHECK(cbuffer);  // TODO(grunell): estimate output size
    if (!cbuffer || buflen == 0) {
        return 0;
    }

    // Init and bounds check.
    unsigned char* bbuffer = reinterpret_cast<unsigned char*>(cbuffer);
    size_t srcpos = 0, bufpos = 0;
    size_t needed = (delimiter) ? (srclen + 1) / 3 : srclen / 2;
    if (buflen < needed) {
        return 0;
    }

    while (srcpos < srclen) {
        if ((srclen - srcpos) < 2) {
            // This means we have an odd number of bytes.
            return 0;
        }

        unsigned char h1, h2;
        if (!hex_decode(source[srcpos], &h1) ||
                !hex_decode(source[srcpos + 1], &h2))
        {
            return 0;
        }
        bbuffer[bufpos++] = (h1 << 4) | h2;
        srcpos += 2;

        // Remove the delimiter if needed.
        if (delimiter && (srclen - srcpos) > 1) {
            if (source[srcpos] != delimiter) {
                return 0;
            }
            ++srcpos;
        }
    }

    return bufpos;
}

size_t split(const std::string& source, char delimiter,
        std::vector<std::string>* fields) 
{
    if (!fields) {
        return 0;
    }
    
    fields->clear();
    size_t last = 0;
    for (size_t i = 0; i < source.length(); ++i) {
        if (source[i] == delimiter) {
            fields->push_back(source.substr(last, i - last));
            last = i + 1;
        }
    }
    fields->push_back(source.substr(last, source.length() - last));
    return fields->size();
}

size_t tokenize(const std::string& source, char delimiter,
        std::vector<std::string>* fields) 
{
    fields->clear();
    size_t last = 0;
    for (size_t i = 0; i < source.length(); ++i) {
        if (source[i] == delimiter) {
            if (i != last) {
                fields->push_back(source.substr(last, i - last));
            }
            last = i + 1;
        }
    }
    if (last != source.length()) {
        fields->push_back(source.substr(last, source.length() - last));
    }
    return fields->size();
}

bool tokenize_first(const std::string& source,
        const char delimiter,
        std::string* token,
        std::string* rest) 
{
    // Find the first delimiter
    size_t left_pos = source.find(delimiter);
    if (left_pos == std::string::npos) {
        return false;
    }

    // Look for additional occurrances of delimiter.
    size_t right_pos = left_pos + 1;
    while (source[right_pos] == delimiter) {
        right_pos++;
    }

    *token = source.substr(0, left_pos);
    *rest = source.substr(right_pos);
    return true;
}

} // namespace rtcbase


