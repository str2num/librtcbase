/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file ref_count_test.cpp
 * @author yujitai
 * @brief 
 *  
 **/

#include <iostream>
using namespace std;

#include <memory>
#include <rtcbase/ref_count.h>
#include <rtcbase/ref_counted_object.h>

class RefClass : public rtcbase::RefCountInterface {
public:
    RefClass() {}
protected:
    ~RefClass() override {}
};

void ref_count_test() {
    cout << "new RefCountedObject<RefClass>." << endl;
    std::unique_ptr<rtcbase::RefCountedObject<RefClass>> ref(
            new rtcbase::RefCountedObject<RefClass>());
    // @remark call manually
    ref->add_ref();
    cout << "ref has one ref -> " << ref->has_one_ref() << endl;

    cout << "another thread x use RefClass." << endl;
    ref->add_ref();
    cout << "ref has one ref -> " << ref->has_one_ref() << endl;

    cout << "thread x release RefClass." << endl;
    const auto status = ref->release();
    if (status == rtcbase::RefCountReleaseStatus::k_dropped_last_ref) {
        cout << "RefCountReleaseStatus::k_dropped_last_ref" << endl;
    } else {
        cout << "RefCountReleaseStatus::k_other_refs_remained" << endl;
    }
    cout << "ref has one ref -> " << ref->has_one_ref() << endl;
}

