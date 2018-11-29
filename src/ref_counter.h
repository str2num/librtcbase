/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ref_counter.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_REF_COUNTER_H_
#define  __RTCBASE_REF_COUNTER_H_

#include "atomicops.h"
#include "ref_count.h"

namespace rtcbase {

class RefCounter {
public:
    explicit RefCounter(int ref_count) : _ref_count(ref_count) {}
    RefCounter() = delete;

    void inc_ref() { rtcbase::AtomicOps::increment(&_ref_count); }

    // TODO(nisse): Switch return type to RefCountReleaseStatus?
    // Returns true if this was the last reference, and the resource protected by
    // the reference counter can be deleted.
    rtcbase::RefCountReleaseStatus dec_ref() {
        return (rtcbase::AtomicOps::decrement(&_ref_count) == 0)
            ? rtcbase::RefCountReleaseStatus::k_dropped_last_ref
            : rtcbase::RefCountReleaseStatus::k_other_refs_remained;
    }

    // Return whether the reference count is one. If the reference count is used
    // in the conventional way, a reference count of 1 implies that the current
    // thread owns the reference and no other thread shares it. This call performs
    // the test for a reference count of one, and performs the memory barrier
    // needed for the owning thread to act on the resource protected by the
    // reference counter, knowing that it has exclusive access.
    bool has_one_ref() const {
        return rtcbase::AtomicOps::acquire_load(&_ref_count) == 1;
    }

private:
    volatile int _ref_count;
};

} // namespace rtcbase

#endif  //__RTCBASE_REF_COUNTER_H_


