/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
/**
 * @file event_loop.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_EVENT_LOOP_H_
#define  __RTCBASE_EVENT_LOOP_H_

#include <string>

struct ev_loop;

namespace rtcbase {

class EventLoop;
class TimerWatcher;
class IOWatcher;

typedef void (*timer_cb_t)(EventLoop *el, TimerWatcher *w, void *priv_data);
typedef void (*io_cb_t)(EventLoop* el, IOWatcher* w, int fd, int revents,
        void* priv_data);

class EventLoop {
public:
    enum {
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x2
    };

    EventLoop(void* owner, bool use_default);
    ~EventLoop();
    
    void run();
    void suspend();
    void resume();
    void stop();
    void sleep(unsigned long usec);

    unsigned long now(); // get current time
    
    // Timer
    TimerWatcher *create_timer(timer_cb_t cb, void *priv_data, bool repeat);
    void start_timer(TimerWatcher *w, unsigned long usec);
    void stop_timer(TimerWatcher *w);
    void delete_timer(TimerWatcher *w);
    
    // io
    IOWatcher* create_io_event(io_cb_t cb, void* priv_data);
    void start_io_event(IOWatcher* w, int fd, int mask);
    void stop_io_event(IOWatcher* w, int fd, int mask);
    void delete_io_event(IOWatcher *w);

    //Global current_time
    static unsigned long current_time();

public:
    void* owner;

private:
    struct ev_loop* _loop;
    // Reference to the default loop, not thread safe, only used for current_time()
    static EventLoop* _default_loop;
};

} // namespace rtcbase

#endif  //__RTCBASE_EVENT_LOOP_H_


