/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file sha1_test.cpp
 * @author yujitai
 * @brief 
 *  
 **/

#include <iostream>
using namespace std;

#include <rtcbase/sha1_digest.h>
#include <rtcbase/string_encode.h>

void sha1_test() {
    rtcbase::Sha1Digest sha1;
    std::string input = "yujitai";
    char output[rtcbase::Sha1Digest::k_size];
    for (size_t i = 0; i < input.size(); ++i) {
        sha1.update(&input[i], 1);
    }
    sha1.finish(output, sizeof(output));
    std::cout << "yujitai -> through hash algorithm sha1 compute -> result:" 
              << rtcbase::hex_encode(output, sizeof(output)) << std::endl;
}

