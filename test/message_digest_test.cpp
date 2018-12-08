/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file message_digest_test.cpp
 * @author yujitai
 * @brief 
 *  
 **/

#include <iostream>
using namespace std;

#include <rtcbase/message_digest.h>
#include <rtcbase/string_encode.h>

void message_digest_test() {
    // compute digest
    // md5
    char output_md5[16];
    rtcbase::compute_digest(rtcbase::DIGEST_MD5, 
            "abc", 3, output_md5, sizeof(output_md5));
    std::cout << "abc -> through hash algorithm md5 -> result:" << 
            rtcbase::hex_encode(output_md5, sizeof(output_md5)) << std::endl;

    // sha1
    char output_sha1[20];
    rtcbase::compute_digest(rtcbase::DIGEST_SHA_1, 
            "abc", 3, output_sha1, sizeof(output_sha1));
    std::cout << "abc -> through hash algorithm sha1 -> result:" << 
            rtcbase::hex_encode(output_sha1, sizeof(output_sha1)) << std::endl;

    // compute hmac
    // hmac md5
    std::string key_md5(16, '\x0b');
    std::string input_md5("Hello World");
    char output_hmac_md5[16];
    rtcbase::compute_hmac(rtcbase::DIGEST_MD5, 
            key_md5.c_str(),
            key_md5.size(),
            input_md5.c_str(),
            input_md5.size(),
            output_hmac_md5,
            sizeof(output_hmac_md5));
    std::cout << "Hello World -> through hash algorithm hmac md5 -> result:" << 
            rtcbase::hex_encode(output_hmac_md5, sizeof(output_hmac_md5)) << std::endl;
    
    // hmac sha1
    std::string key_sha1(20, '\x0b');
    std::string input_sha1("Hello World");
    char output_hmac_sha1[20];
    rtcbase::compute_hmac(rtcbase::DIGEST_SHA_1, 
            key_sha1.c_str(),
            key_sha1.size(),
            input_sha1.c_str(),
            input_sha1.size(),
            output_hmac_sha1,
            sizeof(output_hmac_sha1));
    std::cout << "Hello World -> through hash algorithm hmac sha1 -> result:" << 
            rtcbase::hex_encode(output_hmac_sha1, sizeof(output_hmac_sha1)) << std::endl;
}


