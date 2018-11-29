/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */ 
 
/**
 * @file log_trace_id.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_LOG_TRACE_ID_H_
#define  __RTCBASE_LOG_TRACE_ID_H_

#include <string>

namespace rtcbase {

class LogTraceId {
public:
    std::string get_log_trace_id() {
        return _log_trace_id;
    }
    
    std::string get_log_trace_id() const {
        return _log_trace_id;
    }

    void set_log_trace_id(const std::string& log_trace_id) {
        _log_trace_id = log_trace_id;
    }

private:
    std::string _log_trace_id = "";
};

} // namespace rtcbase

#endif  //__RTCBASE_LOG_TRACE_ID_H_


