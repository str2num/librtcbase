/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
 
/**
 * @file array_size.h
 * @author str2num
 * @brief 
 *  
 **/

#ifndef  __RTCBASE_ARRAY_SIZE_H_
#define  __RTCBASE_ARRAY_SIZE_H_

#include <stddef.h>

// The arraysize(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use arraysize on
// a pointer by mistake, you will get a compile-time error.

// This template function declaration is used in defining arraysize.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N> 
char (&array_size_helper(T (&array)[N]))[N];

#define arraysize(array) (sizeof(array_size_helper(array)))

#endif  //__RTCBASE_ARRAY_SIZE_H_


