/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file memcheck.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_MEMCHECK_H_
#define  __RTCBASE_MEMCHECK_H_

#include <map>

#include "logging.h"

namespace rtcbase {

#ifdef RTCBASE_MEM_CHECK
static std::map<void*, int> g_object_map; 
#endif

class MemCheck {
public:
    MemCheck() {
#ifdef RTCBASE_MEM_CHECK
        this->class_name = "Object";
        LOG(LS_TRACE) << "===============Object construct: " << this;
        g_object_map[this] = 1;
#endif
    }
    
    MemCheck(const std::string& class_name) {
#ifdef RTCBASE_MEM_CHECK
        this->class_name = class_name;
        LOG(LS_TRACE) << "===============[" << class_name << "] construct: " << this;
        g_object_map[this] = 1;
#else
    (void)class_name;
#endif
    }

    virtual ~MemCheck() {
#ifdef RTCBASE_MEM_CHECK
        LOG(LS_TRACE) << "===============[" << class_name << "] destruct: " << this;
        std::map<void*, int>::iterator it;
        it = g_object_map.find(this);
        if (it != g_object_map.end()) {
            g_object_map.erase(it);
        }
        LOG(LS_TRACE) << "============== object_map size: " << g_object_map.size();
        
        // for test
        if (g_object_map.size() >= 6 && g_object_map.size() <= 10) {
            for (it = g_object_map.begin(); it!=g_object_map.end(); ++it) {
                LOG(LS_TRACE) << "============ Not delete: " << it->first;
            }
        }
#endif
    }

public:
#ifdef RTCBASE_MEM_CHECK
    std::string class_name;
#endif
};

} // namespace rtcbase

#endif  //__RTCBASE_MEMCHECK_H_


