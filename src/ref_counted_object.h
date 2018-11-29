/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ref_counted_object.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_REF_COUNTED_OBJECT_H_
#define  __RTCBASE_REF_COUNTED_OBJECT_H_

#include <utility>

#include "constructor_magic.h"
#include "ref_count.h"
#include "ref_counter.h"

namespace rtcbase {

template <class T>
class RefCountedObject : public T {
public:
    RefCountedObject() {}

    template <class P0>
    explicit RefCountedObject(P0&& p0) : T(std::forward<P0>(p0)) {}

    template <class P0, class P1, class... Args>
    RefCountedObject(P0&& p0, P1&& p1, Args&&... args)
        : T(std::forward<P0>(p0),
            std::forward<P1>(p1),
            std::forward<Args>(args)...) 
    {
    }

    virtual void add_ref() const { _ref_count.inc_ref(); }

    virtual RefCountReleaseStatus release() const {
        const auto status = _ref_count.dec_ref();
        if (status == RefCountReleaseStatus::k_dropped_last_ref) {
            delete this;
        }
        return status;
    }

    // Return whether the reference count is one. If the reference count is used
    // in the conventional way, a reference count of 1 implies that the current
    // thread owns the reference and no other thread shares it. This call
    // performs the test for a reference count of one, and performs the memory
    // barrier needed for the owning thread to act on the object, knowing that it
    // has exclusive access to the object.
    virtual bool has_one_ref() const { return _ref_count.has_one_ref(); }

protected:
    virtual ~RefCountedObject() {}

    mutable RefCounter _ref_count{0};

    RTC_DISALLOW_COPY_AND_ASSIGN(RefCountedObject);
};

} // namespace rtcbase

#endif  //__RTCBASE_REF_COUNTED_OBJECT_H_


