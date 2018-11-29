/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file critical_section.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_CRITICAL_SECTION_H_
#define  __RTCBASE_CRITICAL_SECTION_H_

#include <pthread.h>

#include "atomicops.h"
#include "constructor_magic.h"
#include "thread_annotations.h"

namespace rtcbase {

// Locking methods (Enter, TryEnter, Leave)are const to permit protecting
// members inside a const context without requiring mutable CriticalSections
// everywhere.
class LOCKABLE CriticalSection {
public:
    CriticalSection();
    ~CriticalSection();

    void enter() const EXCLUSIVE_LOCK_FUNCTION();
    bool try_enter() const EXCLUSIVE_TRYLOCK_FUNCTION(true);
    void leave() const UNLOCK_FUNCTION();

private:
    mutable pthread_mutex_t _mutex;
};

// CritScope, for serializing execution through a scope.
class SCOPED_LOCKABLE CritScope {
public:
    explicit CritScope(const CriticalSection* cs) EXCLUSIVE_LOCK_FUNCTION(cs);
    ~CritScope() UNLOCK_FUNCTION();
private:
    const CriticalSection* const _cs;
    RTC_DISALLOW_COPY_AND_ASSIGN(CritScope);
};

// A POD lock used to protect global variables. Do NOT use for other purposes.
// No custom constructor or private data member should be added.
class LOCKABLE GlobalLockPod {
public:
    void lock() EXCLUSIVE_LOCK_FUNCTION();

    void unlock() UNLOCK_FUNCTION();

    volatile int lock_acquired;
};

class GlobalLock : public GlobalLockPod {
public:
    GlobalLock();
};

// GlobalLockScope, for serializing execution through a scope.
class SCOPED_LOCKABLE GlobalLockScope {
public:
    explicit GlobalLockScope(GlobalLockPod* lock) EXCLUSIVE_LOCK_FUNCTION(lock);
    ~GlobalLockScope() UNLOCK_FUNCTION();
private:
    GlobalLockPod* const _lock;
    RTC_DISALLOW_COPY_AND_ASSIGN(GlobalLockScope);
};

} // namespace rtcbase

#endif  //__RTCBASE_CRITICAL_SECTION_H_


