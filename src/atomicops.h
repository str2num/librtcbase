/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file atomicops.h
 * @author str2num
 * @brief  
 **/


#ifndef  __RTCBASE_ATOMICOPS_H_
#define  __RTCBASE_ATOMICOPS_H_

namespace rtcbase {

class AtomicOps {
public:
    static int increment(volatile int* i) {
        return __sync_add_and_fetch(i, 1);
    }
    
    static int decrement(volatile int* i) {
        return __sync_sub_and_fetch(i, 1);
    }
    
    static int acquire_load(volatile const int* i) {
        return __atomic_load_n(i, __ATOMIC_ACQUIRE);
    }
    
    static void release_store(volatile int* i, int value) {
        __atomic_store_n(i, value, __ATOMIC_RELEASE);
    }
    
    static int compare_and_swap(volatile int* i, int old_value, int new_value) {
        return __sync_val_compare_and_swap(i, old_value, new_value);
    }
    
    // Pointer variants.
    template <typename T>
    static T* acquire_load_ptr(T* volatile* ptr) {
        return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
    }
    
    template <typename T>
    static T* compare_and_swap_ptr(T* volatile* ptr, T* old_value, T* new_value) {
        return __sync_val_compare_and_swap(ptr, old_value, new_value);
    }
};

} // namespace rtcbase

#endif  //__RTCBASE_ATOMICOPS_H_


