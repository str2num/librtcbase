/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file platform_thread.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <errno.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#include "logging.h"
#include "atomicops.h"
#include "platform_thread.h"

namespace rtcbase {

PlatformThreadId current_thread_id() {
    PlatformThreadId ret;
    // Default implementation for nacl and solaris.
    ret = syscall(__NR_gettid);
    return ret;
}

PlatformThreadRef current_thread_ref() {
    return pthread_self();
}

bool is_thread_ref_equal(const PlatformThreadRef& a, const PlatformThreadRef& b) {
    return pthread_equal(a, b);
}

void set_current_thread_name(const char* name) {
    prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name));
}

namespace {

struct ThreadAttributes {
    ThreadAttributes() { pthread_attr_init(&attr); }
    ~ThreadAttributes() { pthread_attr_destroy(&attr); }
    pthread_attr_t* operator&() { return &attr; }
    pthread_attr_t attr;
};

} // namespace

PlatformThread::PlatformThread(ThreadRunFunctionDeprecated func,
        void* obj,
        const char* thread_name)
    : _run_function_deprecated(func),
    _obj(obj),
    _name(thread_name ? thread_name : "webrtc") 
{
}

PlatformThread::PlatformThread(ThreadRunFunction func,
        void* obj,
        const char* thread_name,
        ThreadPriority priority /*= k_normal_priority*/)
    : _run_function(func), _priority(priority), _obj(obj), _name(thread_name) 
{
}

PlatformThread::~PlatformThread() {
}

// TODO(tommi): Deprecate the loop behavior in PlatformThread.
// * Introduce a new callback type that returns void.
// * Remove potential for a busy loop in PlatformThread.
// * Delegate the responsibility for how to stop the thread, to the
//   implementation that actually uses the thread.
// All implementations will need to be aware of how the thread should be stopped
// and encouraging a busy polling loop, can be costly in terms of power and cpu.
void PlatformThread::run() {
    // Attach the worker thread checker to this thread.
    rtcbase::set_current_thread_name(_name.c_str());

    if (_run_function) {
        set_priority(_priority);
        _run_function(_obj);
        return;
    }

    do {
        // The interface contract of Start/Stop is that for a successful call to
        // Start, there should be at least one call to the run function.  So we
        // call the function before checking |stop_|.
        if (!_run_function_deprecated(_obj)) {
            break;
        }
        static const struct timespec ts_null = {0, 0};
        nanosleep(&ts_null, nullptr);
    } while (!AtomicOps::acquire_load(&_stop_flag));
}

bool PlatformThread::set_priority(ThreadPriority priority) {
    const int policy = SCHED_FIFO;
    const int min_prio = sched_get_priority_min(policy);
    const int max_prio = sched_get_priority_max(policy);
    if (min_prio == -1 || max_prio == -1) {
        return false;
    }

    if (max_prio - min_prio <= 2) {
        return false;
    }

    // Convert webrtc priority to system priorities:
    sched_param param;
    const int top_prio = max_prio - 1;
    const int low_prio = min_prio + 1;
    switch (priority) {
        case k_low_priority:
            param.sched_priority = low_prio;
            break;
        case k_normal_priority:
            // The -1 ensures that the k_high_priority is always greater or equal to
            // k_normal_priority.
            param.sched_priority = (low_prio + top_prio - 1) / 2;
            break;
        case k_high_priority:
            param.sched_priority = std::max(top_prio - 2, low_prio);
            break;
        case k_highest_priority:
            param.sched_priority = std::max(top_prio - 1, low_prio);
            break;
        case k_realtime_priority:
            param.sched_priority = top_prio;
            break;
    }
    return pthread_setschedparam(_thread, policy, &param) == 0;
}

void* PlatformThread::start_thread(void* param) {
    static_cast<PlatformThread*>(param)->run();
    return 0;
}

void PlatformThread::start() {
    ThreadAttributes attr;
    // Set the stack stack size to 1M.
    pthread_attr_setstacksize(&attr, 1024 * 1024);
    if (0 !=pthread_create(&_thread, &attr, &start_thread, this)) {
        LOG(LS_FATAL) << "Create thread failed, errno=" << errno;
    }
}

bool PlatformThread::is_running() const {
    return _thread != 0;
}

void PlatformThread::stop() {
    if (!is_running()) {
        return;
    }
    
    if (!_run_function) {
        AtomicOps::increment(&_stop_flag);
    }

    if (0 != pthread_join(_thread, nullptr)) {
        LOG(LS_FATAL) << "Join thread failed, errno=";
    }
    
    if (!_run_function) {
         AtomicOps::release_store(&_stop_flag, 0);
    }
    
    _thread = 0;
}

} // namespace rtcbase


