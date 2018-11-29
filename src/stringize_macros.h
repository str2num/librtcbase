/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file stringize_macros.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_STRINGIZE_MACROS_H_
#define  __RTCBASE_STRINGIZE_MACROS_H_

// This is not very useful as it does not expand defined symbols if
// called directly. Use its counterpart without the _NO_EXPANSION
// suffix, below.
#define STRINGIZE_NO_EXPANSION(x) #x

// Use this to quote the provided parameter, first expanding it if it
// is a preprocessor symbol.
//
// For example, if:
//   #define A FOO
//   #define B(x) myobj->FunctionCall(x)
//
// Then:
//   STRINGIZE(A) produces "FOO"
//   STRINGIZE(B(y)) produces "myobj->FunctionCall(y)"
#define STRINGIZE(x) STRINGIZE_NO_EXPANSION(x)

#endif  //__RTCBASE_STRINGIZE_MACROS_H_


