/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file sigslot_repeater.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SIGSLOT_REPEATER_H_
#define  __RTCBASE_SIGSLOT_REPEATER_H_

#include "sigslot.h"

namespace rtcbase {

template<class mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
class Repeater0 : public Signal0<mt_policy>,
                  public HasSlots<mt_policy>
{
public:
    typedef Signal0<mt_policy> base_type;
    typedef Repeater0<mt_policy> this_type;

    Repeater0() { }
    Repeater0(const this_type& s) : base_type(s) { }

    void reemit() { Signal0<mt_policy>::emit(); }
    void repeat(base_type &s) { s.connect(this, &this_type::reemit); }
    void stop(base_type &s) { s.disconnect(this); }
};

template<class arg1_type, class mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
class Repeater1 : public Signal1<arg1_type, mt_policy>,
                  public HasSlots<mt_policy>
{
public:
    typedef Signal1<arg1_type, mt_policy> base_type;
    typedef Repeater1<arg1_type, mt_policy> this_type;

    Repeater1() { }
    Repeater1(const this_type& s) : base_type(s) { }

    void reemit(arg1_type a1) { Signal1<arg1_type, mt_policy>::emit(a1); }
    void repeat(base_type& s) { s.connect(this, &this_type::reemit); }
    void stop(base_type &s) { s.disconnect(this); }
};

template<class arg1_type, class arg2_type, class mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
class Repeater2 : public Signal2<arg1_type, arg2_type, mt_policy>,
                  public HasSlots<mt_policy>
{
public:
    typedef Signal2<arg1_type, arg2_type, mt_policy> base_type;
    typedef Repeater2<arg1_type, arg2_type, mt_policy> this_type;

    Repeater2() { }
    Repeater2(const this_type& s) : base_type(s) { }

    void reemit(arg1_type a1, arg2_type a2) { Signal2<arg1_type, arg2_type, mt_policy>::emit(a1,a2); }
    void repeat(base_type& s) { s.connect(this, &this_type::reemit); }
    void stop(base_type &s) { s.disconnect(this); }
};

template<class arg1_type, class arg2_type, class arg3_type,
    class mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
class Repeater3 : public Signal3<arg1_type, arg2_type, arg3_type, mt_policy>,
                  public HasSlots<mt_policy>
{
public:
    typedef Signal3<arg1_type, arg2_type, arg3_type, mt_policy> base_type;
    typedef Repeater3<arg1_type, arg2_type, arg3_type, mt_policy> this_type;

    Repeater3() { }
    Repeater3(const this_type& s) : base_type(s) { }

    void reemit(arg1_type a1, arg2_type a2, arg3_type a3) {
        Signal3<arg1_type, arg2_type, arg3_type, mt_policy>::emit(a1,a2,a3);
    }
    void repeat(base_type& s) { s.connect(this, &this_type::reemit); }
    void stop(base_type &s) { s.disconnect(this); }
};

} // namespace rtcbase

#endif  //__RTCBASE_SIGSLOT_REPEATER_H_


