/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file event.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_EVENT_H_
#define  __RTCBASE_EVENT_H_

#include <pthread.h>
#include "constructor_magic.h"

namespace rtcbase {

class Event {
public:
    static const int k_forever = -1;

    Event(bool manual_reset, bool initially_signaled);
    ~Event();

    void set();
    void reset();

    // Wait for the event to become signaled, for the specified number of
    // |milliseconds|.  To wait indefinetly, pass kForever.
    bool wait(int milliseconds);

private:
    pthread_mutex_t _event_mutex;
    pthread_cond_t _event_cond;
    const bool _is_manual_reset;
    bool _event_status;

    RTC_DISALLOW_IMPLICIT_CONSTRUCTORS(Event);
};

}  // namespace rtcbase

#endif  //__RTCBASE_EVENT_H_


