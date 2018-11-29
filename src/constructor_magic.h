/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file constructor_magic.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_CONSTRUCTOR_MAGIC_H_
#define  __RTCBASE_CONSTRUCTOR_MAGIC_H_

// Put this in the declarations for a class to be unassignable.
#define RTC_DISALLOW_ASSIGN(TypeName) \
    void operator=(const TypeName&) = delete

// A macro to disallow the copy constructor and operator= functions. This should
// be used in the declarations for a class.
#define RTC_DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;        \
    RTC_DISALLOW_ASSIGN(TypeName)

// A macro to disallow all the implicit constructors, namely the default
// constructor, copy constructor and operator= functions.
//
// This should be used in the declarations for a class that wants to prevent
// anyone from instantiating it. This is especially useful for classes
// containing only static methods.
#define RTC_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
    TypeName() = delete;                             \
    RTC_DISALLOW_COPY_AND_ASSIGN(TypeName)

#endif  //__RTCBASE_CONSTRUCTOR_MAGIC_H_


