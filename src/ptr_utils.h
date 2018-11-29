/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file ptr_utils.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_PTR_UTILS_H_
#define  __RTCBASE_PTR_UTILS_H_

#include <memory>

namespace rtcbase {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace rtcbase

#endif  //__RTCBASE_PTR_UTILS_H_


