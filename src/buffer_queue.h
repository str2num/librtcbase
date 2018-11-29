/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file buffer_queue.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_BUFFER_QUEUE_H_
#define  __RTCBASE_BUFFER_QUEUE_H_

#include <deque>
#include <vector>

#include "buffer.h"
#include "constructor_magic.h"
#include "critical_section.h"

namespace rtcbase {

class BufferQueue {
public:
    // Creates a buffer queue with a given capacity and default buffer size.
    BufferQueue(size_t capacity, size_t default_size);
    virtual ~BufferQueue();

    // Return number of queued buffers.
    size_t size() const;

    // Clear the BufferQueue by moving all Buffers from |queue_| to |free_list_|.
    void clear();

    // ReadFront will only read one buffer at a time and will truncate buffers
    // that don't fit in the passed memory.
    // Returns true unless no data could be returned.
    bool read_front(void* data, size_t bytes, size_t* bytes_read);

    // WriteBack always writes either the complete memory or nothing.
    // Returns true unless no data could be written.
    bool write_back(const void* data, size_t bytes, size_t* bytes_written);

protected:
    // These methods are called when the state of the queue changes.
    virtual void notify_readable_for_test() {}
    virtual void notify_writable_for_test() {}

private:
    size_t _capacity;
    size_t _default_size;
    CriticalSection _crit;
    std::deque<Buffer*> _queue GUARDED_BY(_crit);
    std::vector<Buffer*> _free_list GUARDED_BY(_crit);

    RTC_DISALLOW_COPY_AND_ASSIGN(BufferQueue);
};

}  // namespace rtcbase

#endif  //__RTCBASE_BUFFER_QUEUE_H_


