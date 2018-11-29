/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file platform_thread.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_PLATFORM_THREAD_H_
#define  __RTCBASE_PLATFORM_THREAD_H_

#include <string>

#include "constructor_magic.h"
#include "platform_thread_types.h"

namespace rtcbase {

PlatformThreadId current_thread_id();
PlatformThreadRef current_thread_ref();

// Compares two thread identifiers for equality.
bool is_thread_ref_equal(const PlatformThreadRef& a, const PlatformThreadRef& b);

// Sets the current thread name.
void set_current_thread_name(const char* name);

// Callback function that the spawned thread will enter once spawned.
// A return value of false is interpreted as that the function has no
// more work to do and that the thread can be released.
typedef bool (*ThreadRunFunctionDeprecated)(void*);
typedef void (*ThreadRunFunction)(void*);

enum ThreadPriority {
    k_low_priority = 1,
    k_normal_priority = 2,
    k_high_priority = 3,
    k_highest_priority = 4,
    k_realtime_priority = 5
};

// Represents a simple worker thread.  The implementation must be assumed
// to be single threaded, meaning that all methods of the class, must be
// called from the same thread, including instantiation.
class PlatformThread {
public:
    PlatformThread(ThreadRunFunctionDeprecated func,
            void* obj,
            const char* thread_name);
    PlatformThread(ThreadRunFunction func,
            void* obj,
            const char* thread_name,
            ThreadPriority priority = k_normal_priority);
    virtual ~PlatformThread();
   
    const std::string& name() const { return _name; }

    // Spawns a thread and tries to set thread priority according to the priority
    // from when CreateThread was called.
    void start();
    
    bool is_running() const;
    
    // Returns an identifier for the worker thread that can be used to do
    // thread checks.
    PlatformThreadRef get_thread_ref() const; 

    // Stops (joins) the spawned thread.
    void stop();

    // Set the priority of the thread. Must be called when thread is running.
    // TODO(tommi): Make private and only allow public support via ctor.
    bool set_priority(ThreadPriority priority);     

private:
    void run();
    static void* start_thread(void* param);

private:
    ThreadRunFunctionDeprecated const _run_function_deprecated = nullptr;
    ThreadRunFunction const _run_function = nullptr;
    const ThreadPriority _priority = k_normal_priority;
    void* const _obj;
    // TODO(pbos): Make sure call sites use string literals and update to a const
    // char* instead of a std::string.
    const std::string _name;

    volatile int _stop_flag = 0;
    pthread_t _thread = 0;

    RTC_DISALLOW_COPY_AND_ASSIGN(PlatformThread);
};

} // namespace rtcbase

#endif  //__RTCBASE_PLATFORM_THREAD_H_


