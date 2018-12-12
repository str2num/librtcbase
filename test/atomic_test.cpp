/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file atomic_test.cpp
 * @author yujitai
 * @brief 
 *  
 **/

#include <iostream>
using namespace std;

#include <cassert>
#include <thread>
#include <rtcbase/atomicops.h>

#define _USE_ATOMICOPS 1

int global_count = 0;
int value = 0;

void increment() {
    for (int i = 0; i < 100000; i++) {
#if _USE_ATOMICOPS
        rtcbase::AtomicOps::increment(&global_count);
#else
        global_count++;
#endif
    }
}

void decrement() {
    for (int i = 0; i < 100000; i++) {
#if _USE_ATOMICOPS
        rtcbase::AtomicOps::decrement(&global_count);
#else
        global_count--;
#endif
    }
}

void acquire() {
    cout << "AtomicOps::acquire_load(&global_count) -> " 
         << rtcbase::AtomicOps::acquire_load(&global_count) << endl;
}

void release() {
    rtcbase::AtomicOps::release_store(&value, global_count);
    cout << "AtomicOps::release_store(&value, global_count) -> value:" 
         << value << endl;
}

void atomic_test() {
    std::thread thread1(increment); 
    std::thread thread2(decrement); 
    thread1.join();
    thread2.join();
    assert(global_count == 0);
    cout << "global_count:" << global_count << endl;

    global_count = 666;
    std::thread thread3(acquire); 
    std::thread thread4(release); 
    thread3.join();
    thread4.join();
}

