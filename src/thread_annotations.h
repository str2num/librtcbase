/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file thread_annotations.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_THREAD_ANNOTATIONS_H_
#define  __RTCBASE_THREAD_ANNOTATIONS_H_

#define THREAD_ANNOTATION_ATTRIBUTE__(x)  // no-op

// Document if a shared variable/field needs to be protected by a lock.
// GUARDED_BY allows the user to specify a particular lock that should be
// held when accessing the annotated variable, while GUARDED_VAR only
// indicates a shared variable should be guarded (by any lock). GUARDED_VAR
// is primarily used when the client cannot express the name of the lock.
#define GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))
#define GUARDED_VAR THREAD_ANNOTATION_ATTRIBUTE__(guarded)

// Document if a function expects certain locks to be held before it is called
#define EXCLUSIVE_LOCKS_REQUIRED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_locks_required(__VA_ARGS__))

// Document if a class/type is a lockable type (such as the Mutex class).
#define LOCKABLE THREAD_ANNOTATION_ATTRIBUTE__(lockable)

// Document if a class is a scoped lockable type (such as the MutexLock class).
#define SCOPED_LOCKABLE THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

// The following annotations specify lock and unlock primitives.
#define EXCLUSIVE_LOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_lock_function(__VA_ARGS__))

#define EXCLUSIVE_TRYLOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_trylock_function(__VA_ARGS__))

#define UNLOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(unlock_function(__VA_ARGS__))

#endif  //__RTCBASE_THREAD_ANNOTATIONS_H_


