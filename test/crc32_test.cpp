/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file crc32_test.cpp
 * @author yujitai
 * @brief 
 *  
 **/

#include <iostream>
using namespace std;

#include <rtcbase/crc32.h>

void dec2bin(unsigned long n) {
    int r_value;
    r_value = n % 2;
    if (n >= 2) {
        dec2bin(n/2);
    }
    putchar(r_value == 0 ? '0' : '1');
    return;
}

void crc32_test() {
    uint32_t fcs = rtcbase::compute_crc32("abc");
    std::cout << "abc -> crc32 compute -> fcs dec:" << fcs << std::endl; 
    std::cout << "abc -> crc32 compute -> fcs bin:";
    dec2bin(fcs);
    cout << std::endl;

    std::string input = 
            "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    uint32_t c = 0;
    for (size_t i = 0; i < input.size(); ++i) {
        c = rtcbase::update_crc32(c, &input[i], 1);
        std::cout << "update crc32 -> fcs dec:" <<  c << std::endl;
    }
}

