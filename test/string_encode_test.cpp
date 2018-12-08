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

#include <rtcbase/string_encode.h>
#include <string.h>

class HexEncodeTest {
public:
    HexEncodeTest() : _enc_res(0), _dec_res(0) {
        for (size_t i = 0; i < sizeof(_data); ++i) {
            _data[i] = (i + 128) & 0xff;
            std::cout << "origin data:" << (int)_data[i] << std::endl;
        }
        memset(_decoded, 0x7f, sizeof(_decoded));
    }

    char _data[10];
    char _encoded[31];
    char _decoded[11];
    size_t _enc_res;
    size_t _dec_res;
};

void string_encode_test() {
    // single hex encode
    for (size_t i = 0; i < 16; i++) {
        cout << i << " -> hex encode -> result:" 
             << rtcbase::hex_encode(i) << endl;
    }

    // amount hex encode
    HexEncodeTest hex_en;
    hex_en._enc_res = rtcbase::hex_encode(hex_en._encoded, 
            sizeof(hex_en._encoded), 
            hex_en._data, 
            sizeof(hex_en._data));
    std::cout << "hex encoded:" << hex_en._encoded << std::endl;

#if 0
    hex_en._enc_res = rtcbase::hex_encode_with_delimiter(hex_en._encoded, 
            sizeof(hex_en._encoded), 
            hex_en._data, 
            sizeof(hex_en._data),
            ':');
    std::cout << "hex encoded:" << hex_en._encoded << std::endl;
#endif

    hex_en._dec_res = rtcbase::hex_decode(hex_en._decoded, 
            sizeof(hex_en._decoded), 
            hex_en._encoded, 
            hex_en._enc_res);
    for (size_t i = 0; i < 10; i++) {
        std::cout << "hex decoded:" << (int)hex_en._decoded[i] << std::endl;
    }
}


