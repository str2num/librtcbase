/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
 
/**
 * @file byte_buffer.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_BYTE_BUFFER_H_
#define  __RTCBASE_BYTE_BUFFER_H_

#include <string>

#include "memcheck.h"
#include "basic_types.h"
#include "buffer.h"
#include "constructor_magic.h"

namespace rtcbase {

class ByteBuffer : public MemCheck {
public:
    enum ByteOrder {
        ORDER_NETWORK = 0,  // Default, use network byte order (big endian).
        ORDER_HOST,         // Use the native order of the host.
    };

    explicit ByteBuffer(ByteOrder byte_order) : MemCheck("ByteBuffer"), _byte_order(byte_order) {}

    ByteOrder order() const { return _byte_order; }

private:
    ByteOrder _byte_order;

    RTC_DISALLOW_COPY_AND_ASSIGN(ByteBuffer);
};

class ByteBufferWriter : public ByteBuffer {
public:
    // |byte_order| defines order of bytes in the buffer.
    ByteBufferWriter();
    explicit ByteBufferWriter(ByteOrder byte_order);
    ByteBufferWriter(const char* bytes, size_t len);
    ByteBufferWriter(const char* bytes, size_t len, ByteOrder byte_order);

    ~ByteBufferWriter();

    const char* data() const { return _bytes + _start; }
    size_t length() const { return _end - _start; }
    size_t capacity() const { return _size - _start; }

    // Write value to the buffer. Resizes the buffer when it is
    // neccessary.
    void write_uint8(uint8_t val);
    void write_uint16(uint16_t val);
    void write_uint24(uint32_t val);
    void write_uint32(uint32_t val);
    void write_uint64(uint64_t val);
    void write_uvarint(uint64_t val);
    void write_string(const std::string& val);
    void write_bytes(const char* val, size_t len);

    // Reserves the given number of bytes and returns a char* that can be written
    // into. Useful for functions that require a char* buffer and not a
    // ByteBufferWriter.
    char* reserve_write_buffer(size_t len);

    // Resize the buffer to the specified |size|.
    void resize(size_t size);

    // Clears the contents of the buffer. After this, Length() will be 0.
    void clear();

private:
    void construct(const char* bytes, size_t size);

    char* _bytes;
    size_t _size;
    size_t _start;
    size_t _end;

    // There are sensible ways to define these, but they aren't needed in our code
    // base.
    RTC_DISALLOW_COPY_AND_ASSIGN(ByteBufferWriter);
};

// The ByteBufferReader references the passed data, i.e. the pointer must be
// valid during the lifetime of the reader.
class ByteBufferReader : public ByteBuffer {
public:
    ByteBufferReader(const char* bytes, size_t len);
    ByteBufferReader(const char* bytes, size_t len, ByteOrder byte_order);

    // Initializes buffer from a zero-terminated string.
    explicit ByteBufferReader(const char* bytes);

    explicit ByteBufferReader(const Buffer& buf);

    explicit ByteBufferReader(const ByteBufferWriter& buf);

    // Returns start of unprocessed data.
    const char* data() const { return _bytes + _start; }
    // Returns number of unprocessed bytes.
    size_t length() const { return _end - _start; }

    // Read a next value from the buffer. Return false if there isn't
    // enough data left for the specified type.
    bool read_uint8(uint8_t* val);
    bool read_uint16(uint16_t* val);
    bool read_uint24(uint32_t* val);
    bool read_uint32(uint32_t* val);
    bool read_uint64(uint64_t* val);
    bool read_uvarint(uint64_t* val);
    bool read_bytes(char* val, size_t len);

    // Appends next |len| bytes from the buffer to |val|. Returns false
    // if there is less than |len| bytes left.
    bool read_string(std::string* val, size_t len);

    // Moves current position |size| bytes forward. Returns false if
    // there is less than |size| bytes left in the buffer. Consume doesn't
    // permanently remove data, so remembered read positions are still valid
    // after this call.
    bool consume(size_t size);

private:
    void construct(const char* bytes, size_t size);

    const char* _bytes;
    size_t _size;
    size_t _start;
    size_t _end;

    RTC_DISALLOW_COPY_AND_ASSIGN(ByteBufferReader);
};

}  // namespace rtcbase

#endif  //__RTCBASE_BYTE_BUFFER_H_


