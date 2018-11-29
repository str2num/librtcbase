/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */ 
 
/**
 * @file location.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <stdio.h>

#include "location.h"
#include "string_utils.h"

namespace rtcbase {

Location::Location(const char* function_name, const char* file_and_line)
    : _function_name(function_name), _file_and_line(file_and_line) {}

Location::Location() : _function_name("Unknown"), _file_and_line("Unknown") {}

Location::Location(const Location& other)
    : _function_name(other._function_name),
    _file_and_line(other._file_and_line) {}

Location& Location::operator=(const Location& other) {
    _function_name = other._function_name;
    _file_and_line = other._file_and_line;
    return *this;
}

std::string Location::to_string() const {
    char buf[256];
    sprintfn(buf, sizeof(buf), "%s@%s", _function_name, _file_and_line);
    return buf;
}

}  // namespace rtcbase


