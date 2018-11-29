/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file network_test.cpp
 * @author str2num
 * @version $Revision$ 
 * @brief 
 *  
 **/

#include <iostream>

#include <rtcbase/logging.h>
#include <rtcbase/sigslot.h>
#include <rtcbase/network.h>

class TestNetworkManager : public rtcbase::HasSlots<> {
public:
    TestNetworkManager();
    void on_networks_changed();

private:
    std::unique_ptr<rtcbase::NetworkManager> _mgr;
};

TestNetworkManager::TestNetworkManager() :
    _mgr(new rtcbase::BasicNetworkManager())
{
    _mgr->signal_networks_changed.connect(this, &TestNetworkManager::on_networks_changed);
    _mgr->start_updating();
}

void TestNetworkManager::on_networks_changed() {
    _mgr->dump_networks();
}

void test_create_networks() {
    TestNetworkManager t;
}


