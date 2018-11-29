/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ref_counted_base.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_REF_COUNTED_BASE_H_
#define  __RTCBASE_REF_COUNTED_BASE_H_

#include "constructor_magic.h"
#include "ref_count.h"
#include "ref_counter.h"

namespace rtcbase {

class RefCountedBase {
public:
    RefCountedBase() = default;

    void add_ref() const { _ref_count.inc_ref(); }
    RefCountReleaseStatus release() const {
        const auto status = _ref_count.dec_ref();
        if (status == RefCountReleaseStatus::k_dropped_last_ref) {
            delete this;
        }
        return status;
    }

protected:
    virtual ~RefCountedBase() = default;

private:
    mutable RefCounter _ref_count{0};

    RTC_DISALLOW_COPY_AND_ASSIGN(RefCountedBase);
};

}  // namespace rtcbase

#endif  //__RTCBASE_REF_COUNTED_BASE_H_


