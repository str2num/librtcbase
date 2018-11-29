/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
 
/**
 * @file buffer_queue.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <algorithm>

#include "buffer_queue.h"

namespace rtcbase {

BufferQueue::BufferQueue(size_t capacity, size_t default_size)
    : _capacity(capacity), _default_size(default_size) 
{
}

BufferQueue::~BufferQueue() {
    CritScope cs(&_crit);

    for (Buffer* buffer : _queue) {
        delete buffer;
    }
    for (Buffer* buffer : _free_list) {
        delete buffer;
    }
}

size_t BufferQueue::size() const {
    CritScope cs(&_crit);
    return _queue.size();
}

void BufferQueue::clear() {
    CritScope cs(&_crit);
    while (!_queue.empty()) {
        _free_list.push_back(_queue.front());
        _queue.pop_front();
    }
}

bool BufferQueue::read_front(void* buffer, size_t bytes, size_t* bytes_read) {
    CritScope cs(&_crit);
    if (_queue.empty()) {
        return false;
    }

    bool was_writable = _queue.size() < _capacity;
    Buffer* packet = _queue.front();
    _queue.pop_front();

    bytes = std::min(bytes, packet->size());
    memcpy(buffer, packet->data(), bytes);
    if (bytes_read) {
        *bytes_read = bytes;
    }
    _free_list.push_back(packet);
    if (!was_writable) {
        notify_writable_for_test();
    }
    return true;
}

bool BufferQueue::write_back(const void* buffer, size_t bytes,
        size_t* bytes_written) 
{
    CritScope cs(&_crit);
    if (_queue.size() == _capacity) {
        return false;
    }

    bool was_readable = !_queue.empty();
    Buffer* packet;
    if (!_free_list.empty()) {
        packet = _free_list.back();
        _free_list.pop_back();
    } else {
        packet = new Buffer(bytes, _default_size);
    }

    packet->set_data(static_cast<const uint8_t*>(buffer), bytes);
    if (bytes_written) {
        *bytes_written = bytes;
    }
    _queue.push_back(packet);
    if (!was_readable) {
        notify_readable_for_test();
    }
    return true;
}

}  // namespace rtcbase


