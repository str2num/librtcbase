/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file byte_buffer.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <assert.h>
#include <string.h>

#include <algorithm>

#include "basic_types.h"
#include "byte_order.h"
#include "byte_buffer.h"

namespace rtcbase {

static const int DEFAULT_SIZE = 4096;

///////////////////// ByteBufferWriter //////////////////

ByteBufferWriter::ByteBufferWriter()
    : ByteBuffer(ORDER_NETWORK) 
{
    construct(NULL, DEFAULT_SIZE);
}

ByteBufferWriter::ByteBufferWriter(ByteOrder byte_order)
    : ByteBuffer(byte_order) 
{
    construct(NULL, DEFAULT_SIZE);
}

ByteBufferWriter::ByteBufferWriter(const char* bytes, size_t len)
    : ByteBuffer(ORDER_NETWORK) 
{
    construct(bytes, len);
}

ByteBufferWriter::ByteBufferWriter(const char* bytes, size_t len,
        ByteOrder byte_order)
    : ByteBuffer(byte_order) 
{
    construct(bytes, len);
}

void ByteBufferWriter::construct(const char* bytes, size_t len) {
    _start = 0;
    _size = len;
    _bytes = new char[_size];

    if (bytes) {
        _end = len;
        memcpy(_bytes, bytes, _end);
    } else {
        _end = 0;
    }
}

ByteBufferWriter::~ByteBufferWriter() {
    delete[] _bytes;
}

void ByteBufferWriter::write_uint8(uint8_t val) {
    write_bytes(reinterpret_cast<const char*>(&val), 1);
}

void ByteBufferWriter::write_uint16(uint16_t val) {
    uint16_t v = (order() == ORDER_NETWORK) ? host_to_network16(val) : val;
    write_bytes(reinterpret_cast<const char*>(&v), 2);
}

void ByteBufferWriter::write_uint24(uint32_t val) {
    uint32_t v = (order() == ORDER_NETWORK) ? host_to_network32(val) : val;
    char* start = reinterpret_cast<char*>(&v);
    if (order() == ORDER_NETWORK || is_host_big_endian()) {
        ++start;
    }
    write_bytes(start, 3);
}

void ByteBufferWriter::write_uint32(uint32_t val) {
    uint32_t v = (order() == ORDER_NETWORK) ? host_to_network32(val) : val;
    write_bytes(reinterpret_cast<const char*>(&v), 4);
}

void ByteBufferWriter::write_uint64(uint64_t val) {
    uint64_t v = (order() == ORDER_NETWORK) ? host_to_network64(val) : val;
    write_bytes(reinterpret_cast<const char*>(&v), 8);
}

// Serializes an unsigned varint in the format described by
// https://developers.google.com/protocol-buffers/docs/encoding#varints
// with the caveat that integers are 64-bit, not 128-bit.
void ByteBufferWriter::write_uvarint(uint64_t val) {
    while (val >= 0x80) {
        // Write 7 bits at a time, then set the msb to a continuation byte (msb=1).
        char byte = static_cast<char>(val) | 0x80;
        write_bytes(&byte, 1);
        val >>= 7;
    }
    char last_byte = static_cast<char>(val);
    write_bytes(&last_byte, 1);
}

void ByteBufferWriter::write_string(const std::string& val) {
    write_bytes(val.c_str(), val.size());
}

void ByteBufferWriter::write_bytes(const char* val, size_t len) {
    memcpy(reserve_write_buffer(len), val, len);
}

char* ByteBufferWriter::reserve_write_buffer(size_t len) {
    if (length() + len > capacity()) {
        resize(length() + len);
    }

    char* start = _bytes + _end;
    _end += len;
    return start;
}

void ByteBufferWriter::resize(size_t size) {
    size_t len = std::min(_end - _start, size);
    if (size <= _size) {
        // Don't reallocate, just move data backwards
        memmove(_bytes, _bytes + _start, len);
    } else {
        // Reallocate a larger buffer.
        _size = std::max(size, 3 * _size / 2);
        char* new_bytes = new char[_size];
        memcpy(new_bytes, _bytes + _start, len);
        delete [] _bytes;
        _bytes = new_bytes;
    }
    _start = 0;
    _end = len;
}

void ByteBufferWriter::clear() {
    memset(_bytes, 0, _size);
    _start = _end = 0;
}

////////////////////// ByteBufferReader ////////////////////////

ByteBufferReader::ByteBufferReader(const char* bytes, size_t len)
    : ByteBuffer(ORDER_NETWORK) 
{
    construct(bytes, len);
}

ByteBufferReader::ByteBufferReader(const char* bytes, size_t len,
        ByteOrder byte_order)
    : ByteBuffer(byte_order) 
{
    construct(bytes, len);
}

ByteBufferReader::ByteBufferReader(const char* bytes)
    : ByteBuffer(ORDER_NETWORK) 
{
    construct(bytes, strlen(bytes));
}

ByteBufferReader::ByteBufferReader(const Buffer& buf)
    : ByteBuffer(ORDER_NETWORK) 
{
    construct(buf.data<char>(), buf.size());
}

ByteBufferReader::ByteBufferReader(const ByteBufferWriter& buf)
    : ByteBuffer(buf.order()) 
{
    construct(buf.data(), buf.length());
}

void ByteBufferReader::construct(const char* bytes, size_t len) {
    _bytes = bytes;
    _size = len;
    _start = 0;
    _end = len;
}

bool ByteBufferReader::read_uint8(uint8_t* val) {
  if (!val) {
      return false;
  }

  return read_bytes(reinterpret_cast<char*>(val), 1);
}

bool ByteBufferReader::read_uint16(uint16_t* val) {
    if (!val) {
        return false;
    }

    uint16_t v;
    if (!read_bytes(reinterpret_cast<char*>(&v), 2)) {
        return false;
    } else {
        *val = (order() == ORDER_NETWORK) ? network_to_host16(v) : v;
        return true;
    }
}

bool ByteBufferReader::read_uint24(uint32_t* val) {
    if (!val) {
        return false;
    }

    uint32_t v = 0;
    char* read_into = reinterpret_cast<char*>(&v);
    if (order() == ORDER_NETWORK || is_host_big_endian()) {
        ++read_into;
    }

    if (!read_bytes(read_into, 3)) {
        return false;
    } else {
        *val = (order() == ORDER_NETWORK) ? network_to_host32(v) : v;
        return true;
    }
}

bool ByteBufferReader::read_uint32(uint32_t* val) {
    if (!val) {
        return false;
    }

    uint32_t v;
    if (!read_bytes(reinterpret_cast<char*>(&v), 4)) {
        return false;
    } else {
        *val = (order() == ORDER_NETWORK) ? network_to_host32(v) : v;
        return true;
    }
}

bool ByteBufferReader::read_uint64(uint64_t* val) {
    if (!val) {
        return false;
    }

    uint64_t v;
    if (!read_bytes(reinterpret_cast<char*>(&v), 8)) {
        return false;
    } else {
        *val = (order() == ORDER_NETWORK) ? network_to_host64(v) : v;
        return true;
    }
}

bool ByteBufferReader::read_uvarint(uint64_t* val) {
    if (!val) {
        return false;
    }
    // Integers are deserialized 7 bits at a time, with each byte having a
    // continuation byte (msb=1) if there are more bytes to be read.
    uint64_t v = 0;
    for (int i = 0; i < 64; i += 7) {
        char byte;
        if (!read_bytes(&byte, 1)) {
            return false;
        }
        // Read the first 7 bits of the byte, then offset by bits read so far.
        v |= (static_cast<uint64_t>(byte) & 0x7F) << i;
        // True if the msb is not a continuation byte.
        if (static_cast<uint64_t>(byte) < 0x80) {
            *val = v;
            return true;
        }
    }
    return false;
}

bool ByteBufferReader::read_string(std::string* val, size_t len) {
    if (!val) {
        return false;
    }

    if (len > length()) {
        return false;
    } else {
        val->append(_bytes + _start, len);
        _start += len;
        return true;
    }
}

bool ByteBufferReader::read_bytes(char* val, size_t len) {
    if (len > length()) {
        return false;
    } else {
        memcpy(val, _bytes + _start, len);
        _start += len;
        return true;
    }
}

bool ByteBufferReader::consume(size_t size) {
    if (size > length()) {
        return false;
    }
    _start += size;
    return true;
}

}  // namespace rtcbase


