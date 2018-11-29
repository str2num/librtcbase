/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file array_size_test.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <iostream>

#include <rtcbase/array_size.h>

void test_array_size() {
    int a[] = {1, 2, 3, 4};
    char b[] = {'a', 'b'};

    std::cout << "sizeof(a) = " << arraysize(a) << std::endl;
    std::cout << "sizeof(b) = " << arraysize(b) << std::endl;
}

















