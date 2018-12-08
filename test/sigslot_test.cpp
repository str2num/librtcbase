/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file sigslot_test.cpp
 * @author yujitai
 * @brief 
 *  
 **/

#include <rtcbase/sigslot.h>
#include <string>  
#include <unistd.h>
#include <iostream>  
using namespace std;  
  
class CSender  
{  
public:  
    rtcbase::Signal2<string, int> signal_in_danger;  
  
    void sos()  
    {  
        static int num = 0;  
        char signal[20] = {0};  
        sprintf(signal, "help me...");  
        signal_in_danger(signal, num++);  
        // signal_in_danger.emit(signal, num++);  
    }  
};  
  
class CReceiver : public rtcbase::HasSlots<>  
{  
public:  
    void on_danger(string msg, int num)  
    {  
        cout << msg.c_str() << " ==> " << num << endl;  
    }  
};  
  
void sigslot_test()  
{  
    CSender sender;  
    CReceiver receiver;  
    cout << "create object ok..." << endl;  

    sender.signal_in_danger.connect(&receiver, &CReceiver::on_danger);  
    cout << "connect signal to slot success..." << endl;  

    while (1)  
    {  
        sender.sos();  
        sleep(1);  
    }  
}

