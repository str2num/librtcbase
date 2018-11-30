/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file base64_test.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <iostream>

#include <rtcbase/base64.h>

void test_base64_decode();
void test_base64_encode();

void test_base64() {
    std::string str = "hello world!";
    std::string encode_str = rtcbase::Base64::encode(str);
    std::cout << "encode: " << str << "-->" << encode_str << std::endl;
    std::string decode_str = rtcbase::Base64::decode(encode_str, rtcbase::Base64::DO_LAX);
    std::cout << "decode: " << encode_str << "-->" << decode_str << std::endl;
}


