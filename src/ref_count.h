/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ref_count.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_REF_COUNT_H_
#define  __RTCBASE_REF_COUNT_H_

namespace rtcbase {

enum class RefCountReleaseStatus { k_dropped_last_ref, k_other_refs_remained };

class RefCountInterface {
public:
    virtual void add_ref() const = 0;
    virtual RefCountReleaseStatus release() const = 0;

    // Non-public destructor, because Release() has exclusive responsibility for
    // destroying the object.
protected:
    virtual ~RefCountInterface() {}
};

} // namespace rtcbase

#endif  //__RTCBASE_REF_COUNT_H_


