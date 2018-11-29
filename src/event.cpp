/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
/**
 * @file event.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include "event.h"

#include <pthread.h>
#include <sys/time.h>
#include <time.h>

namespace rtcbase {

Event::Event(bool manual_reset, bool initially_signaled)
    : _is_manual_reset(manual_reset),
      _event_status(initially_signaled) 
{
    pthread_mutex_init(&_event_mutex, nullptr);
    pthread_cond_init(&_event_cond, nullptr);
}

Event::~Event() {
    pthread_mutex_destroy(&_event_mutex);
    pthread_cond_destroy(&_event_cond);
}

void Event::set() {
    pthread_mutex_lock(&_event_mutex);
    _event_status = true;
    pthread_cond_broadcast(&_event_cond);
    pthread_mutex_unlock(&_event_mutex);
}

void Event::reset() {
    pthread_mutex_lock(&_event_mutex);
    _event_status = false;
    pthread_mutex_unlock(&_event_mutex);
}

bool Event::wait(int milliseconds) {
    int error = 0;

    struct timespec ts;
    if (milliseconds != k_forever) {
        // Converting from seconds and microseconds (1e-6) plus
        // milliseconds (1e-3) to seconds and nanoseconds (1e-9).

#ifdef HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE
        // Use relative time version, which tends to be more efficient for
        // pthread implementations where provided (like on Android).
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000;
#else
        struct timeval tv;
        gettimeofday(&tv, nullptr);

        ts.tv_sec = tv.tv_sec + (milliseconds / 1000);
        ts.tv_nsec = tv.tv_usec * 1000 + (milliseconds % 1000) * 1000000;

        // Handle overflow.
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
#endif
    }

    pthread_mutex_lock(&_event_mutex);
    if (milliseconds != k_forever) {
        while (!_event_status && error == 0) {
#ifdef HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE
            error = pthread_cond_timedwait_relative_np(
                    &_event_cond, &_event_mutex, &ts);
#else
            error = pthread_cond_timedwait(&_event_cond, &_event_mutex, &ts);
#endif
        }
    } else {
        while (!_event_status && error == 0) {
            error = pthread_cond_wait(&_event_cond, &_event_mutex);
        }
    }

    // NOTE(liulk): Exactly one thread will auto-reset this event. All
    // the other threads will think it's unsignaled.  This seems to be
    // consistent with auto-reset events in WEBRTC_WIN
    if (error == 0 && !_is_manual_reset) {
        _event_status = false;
    }

    pthread_mutex_unlock(&_event_mutex);

    return (error == 0);
}

}  // namespace rtcbase


