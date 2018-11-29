/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file stream.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "stream.h"

namespace rtcbase {

StreamResult StreamInterface::write_all(const void* data, size_t data_len,
        size_t* written, int* error) 
{
    StreamResult result = SR_SUCCESS;
    size_t total_written = 0, current_written;
    while (total_written < data_len) {
        result = write(static_cast<const char*>(data) + total_written,
                data_len - total_written, &current_written, error);
        if (result != SR_SUCCESS) {
            break;
        }
        total_written += current_written;
    }
    if (written) {
        *written = total_written;
    }
    return result;
}

StreamAdapterInterface::StreamAdapterInterface(StreamInterface* stream,
        bool owned)
    : _stream(stream), _owned(owned) 
{
    if (NULL != _stream) {
        _stream->signal_event.connect(this, &StreamAdapterInterface::on_event);
    }
}

StreamAdapterInterface::~StreamAdapterInterface() {
    if (_owned) {
        delete _stream;
    }
}

void StreamAdapterInterface::on_event(StreamInterface* stream,
        int events,
        int err) 
{
    (void)stream;
    signal_event(this, events, err);
}

StreamState StreamAdapterInterface::get_state() const {
    return _stream->get_state();
}

StreamResult StreamAdapterInterface::read(void* buffer,
        size_t buffer_len,
        size_t* read,
        int* error) 
{
    return _stream->read(buffer, buffer_len, read, error);
}

StreamResult StreamAdapterInterface::write(const void* data,
        size_t data_len,
        size_t* written,
        int* error) 
{
    return _stream->write(data, data_len, written, error);
}

} // namespace rtcbase


