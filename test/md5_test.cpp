/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file md5_test.cpp
 * @author yujitai
 * @brief 
 *  
 **/

#include <iostream>
using namespace std;

#include <rtcbase/md5_digest.h>
#include <rtcbase/string_encode.h>

void md5_test() {
    rtcbase::Md5Digest md5;
    std::string input = "yujitai";
    char output[rtcbase::Md5Digest::k_size];

    for (size_t i = 0; i < input.size(); ++i) {
        md5.update(&input[i], 1);
    }
    md5.finish(output, sizeof(output));

    std::cout << "yujitai -> through hash algorithm md5 compute -> result:" 
              << rtcbase::hex_encode(output, sizeof(output)) << std::endl;
}


