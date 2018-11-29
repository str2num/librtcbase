/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file critical_section.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "logging.h"
#include "critical_section.h"

namespace rtcbase {

CriticalSection::CriticalSection() {
    pthread_mutexattr_t mutex_attribute;
    pthread_mutexattr_init(&mutex_attribute);
    pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_mutex, &mutex_attribute);
    pthread_mutexattr_destroy(&mutex_attribute);
}

CriticalSection::~CriticalSection() {
    pthread_mutex_destroy(&_mutex);
}

void CriticalSection::enter() const EXCLUSIVE_LOCK_FUNCTION() {
    pthread_mutex_lock(&_mutex);
}

bool CriticalSection::try_enter() const EXCLUSIVE_TRYLOCK_FUNCTION(true) {
    if (pthread_mutex_trylock(&_mutex) != 0) {
        return false;
    }
    return true;
}

void CriticalSection::leave() const UNLOCK_FUNCTION() {
    pthread_mutex_unlock(&_mutex);
}

CritScope::CritScope(const CriticalSection* cs) : _cs(cs) { _cs->enter(); }
CritScope::~CritScope() { _cs->leave(); }

void GlobalLockPod::lock() {
    const struct timespec ts_null = {0, 0};
    while (AtomicOps::compare_and_swap(&lock_acquired, 0, 1)) {
        nanosleep(&ts_null, nullptr);
    }
}

void GlobalLockPod::unlock() {
    int old_value = AtomicOps::compare_and_swap(&lock_acquired, 1, 0);
    if (old_value != 1) {
        LOG(LS_WARNING) << "Unlock called without calling Lock first";
    }
}

GlobalLock::GlobalLock() {
    lock_acquired = 0;
}

GlobalLockScope::GlobalLockScope(GlobalLockPod* lock)
    : _lock(lock) 
{
    _lock->lock();
}

GlobalLockScope::~GlobalLockScope() {
    _lock->unlock();
}

} // namespace rtcbase


