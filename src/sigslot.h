/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */ 
 
/**
 * @file sigslot.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_SIGSLOT_H_
#define  __RTCBASE_SIGSLOT_H_

#include <stdlib.h>
#include <cstring>
#include <list>
#include <set>

#include <pthread.h>

// On our copy of sigslot.h, we set single threading as default.
#define SIGSLOT_DEFAULT_MT_POLICY SingleThreaded

#ifndef SIGSLOT_DEFAULT_MT_POLICY
#ifdef _SIGSLOT_SINGLE_THREADED
#define SIGSLOT_DEFAULT_MT_POLICY SingleThreaded
#else
#define SIGSLOT_DEFAULT_MT_POLICY MultiThreadedLocal
#endif
#endif

namespace rtcbase {

class SingleThreaded {
public:
    void lock() {}
    void unlock() {}
};

// The multi threading policies only get compiled in if they are enabled.
class MultiThreadedGlobal {
public:
    void lock() { pthread_mutex_lock(get_mutex()); }
    void unlock() { pthread_mutex_unlock(get_mutex()); }

private:
    static pthread_mutex_t* get_mutex();
};

class MultiThreadedLocal {
public:
    MultiThreadedLocal() { pthread_mutex_init(&m_mutex, nullptr); }
    MultiThreadedLocal(const MultiThreadedLocal&) {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~MultiThreadedLocal() { pthread_mutex_destroy(&m_mutex); }
    void lock() { pthread_mutex_lock(&m_mutex); }
    void unlock() { pthread_mutex_unlock(&m_mutex); }

private:
    pthread_mutex_t m_mutex;
};

template <class mt_policy>
class LockBlock {
public:
    mt_policy* m_mutex;
    LockBlock(mt_policy* mtx) : m_mutex(mtx) { m_mutex->lock(); }
    ~LockBlock() { m_mutex->unlock(); }
};

class _SignalBaseInterface;

class HasSlotsInterface {
private:
    typedef void (*signal_connect_t)(HasSlotsInterface* self,
            _SignalBaseInterface* sender);
    typedef void (*signal_disconnect_t)(HasSlotsInterface* self,
            _SignalBaseInterface* sender);
    typedef void (*disconnect_all_t)(HasSlotsInterface* self);

    const signal_connect_t m_signal_connect;
    const signal_disconnect_t m_signal_disconnect;
    const disconnect_all_t m_disconnect_all;

protected:
    HasSlotsInterface(signal_connect_t conn,
            signal_disconnect_t disc,
            disconnect_all_t disc_all)
        : m_signal_connect(conn),
        m_signal_disconnect(disc),
        m_disconnect_all(disc_all) {}

    // Doesn't really need to be virtual, but is for backwards compatibility
    // (it was virtual in a previous version of sigslot).
    virtual ~HasSlotsInterface() {}

public:
    void signal_connect(_SignalBaseInterface* sender) {
        m_signal_connect(this, sender);
    }

    void signal_disconnect(_SignalBaseInterface* sender) {
        m_signal_disconnect(this, sender);
    }

    void disconnect_all() { m_disconnect_all(this); }
};

class _SignalBaseInterface {
private:
    typedef void (*slot_disconnect_t)(_SignalBaseInterface* self,
            HasSlotsInterface* pslot);
    typedef void (*slot_duplicate_t)(_SignalBaseInterface* self,
            const HasSlotsInterface* poldslot,
            HasSlotsInterface* pnewslot);

    const slot_disconnect_t m_slot_disconnect;
    const slot_duplicate_t m_slot_duplicate;

protected:
    _SignalBaseInterface(slot_disconnect_t disc, slot_duplicate_t dupl)
        : m_slot_disconnect(disc), m_slot_duplicate(dupl) {}

    ~_SignalBaseInterface() {}

public:
    void slot_disconnect(HasSlotsInterface* pslot) {
        m_slot_disconnect(this, pslot);
    }

    void slot_duplicate(const HasSlotsInterface* poldslot,
            HasSlotsInterface* pnewslot) {
        m_slot_duplicate(this, poldslot, pnewslot);
    }
};

class _OpaqueConnection {
private:
    typedef void (*emit_t)(const _OpaqueConnection*);
    template <typename FromT, typename ToT>
        union union_caster {
            FromT from;
            ToT to;
        };

    emit_t pemit;
    HasSlotsInterface* pdest;
    // Pointers to member functions may be up to 16 bytes for virtual classes,
    // so make sure we have enough space to store it.
    unsigned char pmethod[16];

public:
    template <typename DestT, typename... Args>
    _OpaqueConnection(DestT* pd, void (DestT::*pm)(Args...)) : pdest(pd) {
        typedef void (DestT::*pm_t)(Args...);
        static_assert(sizeof(pm_t) <= sizeof(pmethod),
                "Size of slot function pointer too large.");

        std::memcpy(pmethod, &pm, sizeof(pm_t));

        typedef void (*em_t)(const _OpaqueConnection* self, Args...);
        union_caster<em_t, emit_t> caster2;
        caster2.from = &_OpaqueConnection::emitter<DestT, Args...>;
        pemit = caster2.to;
    }

    HasSlotsInterface* getdest() const { return pdest; }

    _OpaqueConnection duplicate(HasSlotsInterface* newtarget) const {
        _OpaqueConnection res = *this;
        res.pdest = newtarget;
        return res;
    }

    // Just calls the stored "emitter" function pointer stored at construction
    // time.
    template <typename... Args>
    void emit(Args... args) const {
        typedef void (*em_t)(const _OpaqueConnection*, Args...);
        union_caster<emit_t, em_t> caster;
        caster.from = pemit;
        (caster.to)(this, args...);
    }

private:
    template <typename DestT, typename... Args>
    static void emitter(const _OpaqueConnection* self, Args... args) {
        typedef void (DestT::*pm_t)(Args...);
        pm_t pm;
        std::memcpy(&pm, self->pmethod, sizeof(pm_t));
        (static_cast<DestT*>(self->pdest)->*(pm))(args...);
    }
};

template <class mt_policy>
class _SignalBase : public _SignalBaseInterface, public mt_policy {
protected:
    typedef std::list<_OpaqueConnection> connections_list;

    _SignalBase()
        : _SignalBaseInterface(&_SignalBase::do_slot_disconnect,
                &_SignalBase::do_slot_duplicate),
        m_current_iterator(m_connected_slots.end()) {}

    ~_SignalBase() { disconnect_all(); }

private:
    _SignalBase& operator=(_SignalBase const& that);

public:
    _SignalBase(const _SignalBase& o)
        : _SignalBaseInterface(&_SignalBase::do_slot_disconnect,
                &_SignalBase::do_slot_duplicate),
        m_current_iterator(m_connected_slots.end()) {
            LockBlock<mt_policy> lock(this);
            for (const auto& connection : o.m_connected_slots) {
                connection.getdest()->signal_connect(this);
                m_connected_slots.push_back(connection);
            }
        }

    bool is_empty() {
        LockBlock<mt_policy> lock(this);
        return m_connected_slots.empty();
    }

    void disconnect_all() {
        LockBlock<mt_policy> lock(this);

        while (!m_connected_slots.empty()) {
            HasSlotsInterface* pdest = m_connected_slots.front().getdest();
            m_connected_slots.pop_front();
            pdest->signal_disconnect(static_cast<_SignalBaseInterface*>(this));
        }
        // If disconnect_all is called while the signal is firing, advance the
        // current slot iterator to the end to avoid an invalidated iterator from
        // being dereferenced.
        m_current_iterator = m_connected_slots.end();
    }

    void disconnect(HasSlotsInterface* pclass) {
        LockBlock<mt_policy> lock(this);
        connections_list::iterator it = m_connected_slots.begin();
        connections_list::iterator itEnd = m_connected_slots.end();

        while (it != itEnd) {
            if (it->getdest() == pclass) {
                // If we're currently using this iterator because the signal is firing,
                // advance it to avoid it being invalidated.
                if (m_current_iterator == it) {
                    m_current_iterator = m_connected_slots.erase(it);
                } else {
                    m_connected_slots.erase(it);
                }
                pclass->signal_disconnect(static_cast<_SignalBaseInterface*>(this));
                return;
            }
            ++it;
        }
    }

private:
    static void do_slot_disconnect(_SignalBaseInterface* p,
            HasSlotsInterface* pslot) {
        _SignalBase* const self = static_cast<_SignalBase*>(p);
        LockBlock<mt_policy> lock(self);
        connections_list::iterator it = self->m_connected_slots.begin();
        connections_list::iterator itEnd = self->m_connected_slots.end();

        while (it != itEnd) {
            connections_list::iterator itNext = it;
            ++itNext;

            if (it->getdest() == pslot) {
                // If we're currently using this iterator because the signal is firing,
                // advance it to avoid it being invalidated.
                if (self->m_current_iterator == it) {
                    self->m_current_iterator = self->m_connected_slots.erase(it);
                } else {
                    self->m_connected_slots.erase(it);
                }
            }

            it = itNext;
        }
    }

    static void do_slot_duplicate(_SignalBaseInterface* p,
            const HasSlotsInterface* oldtarget,
            HasSlotsInterface* newtarget) {
        _SignalBase* const self = static_cast<_SignalBase*>(p);
        LockBlock<mt_policy> lock(self);
        connections_list::iterator it = self->m_connected_slots.begin();
        connections_list::iterator itEnd = self->m_connected_slots.end();

        while (it != itEnd) {
            if (it->getdest() == oldtarget) {
                self->m_connected_slots.push_back(it->duplicate(newtarget));
            }

            ++it;
        }
    }

protected:
    connections_list m_connected_slots;

    // Used to handle a slot being disconnected while a signal is
    // firing (iterating m_connected_slots).
    connections_list::iterator m_current_iterator;
    bool m_erase_current_iterator = false;
};

template <class mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
class HasSlots : public HasSlotsInterface, public mt_policy {
private:
    typedef std::set<_SignalBaseInterface*> sender_set;
    typedef sender_set::const_iterator const_iterator;

public:
    HasSlots()
        : HasSlotsInterface(&HasSlots::do_signal_connect,
                &HasSlots::do_signal_disconnect,
                &HasSlots::do_disconnect_all) {}

    HasSlots(HasSlots const& o)
        : HasSlotsInterface(&HasSlots::do_signal_connect,
                &HasSlots::do_signal_disconnect,
                &HasSlots::do_disconnect_all) {
            LockBlock<mt_policy> lock(this);
            for (auto* sender : o.m_senders) {
                sender->slot_duplicate(&o, this);
                m_senders.insert(sender);
            }
        }

    ~HasSlots() { this->disconnect_all(); }

private:
    HasSlots& operator=(HasSlots const&);

    static void do_signal_connect(HasSlotsInterface* p,
            _SignalBaseInterface* sender) {
        HasSlots* const self = static_cast<HasSlots*>(p);
        LockBlock<mt_policy> lock(self);
        self->m_senders.insert(sender);
    }

    static void do_signal_disconnect(HasSlotsInterface* p,
            _SignalBaseInterface* sender) {
        HasSlots* const self = static_cast<HasSlots*>(p);
        LockBlock<mt_policy> lock(self);
        self->m_senders.erase(sender);
    }

    static void do_disconnect_all(HasSlotsInterface* p) {
        HasSlots* const self = static_cast<HasSlots*>(p);
        LockBlock<mt_policy> lock(self);
        while (!self->m_senders.empty()) {
            std::set<_SignalBaseInterface*> senders;
            senders.swap(self->m_senders);
            const_iterator it = senders.begin();
            const_iterator itEnd = senders.end();

            while (it != itEnd) {
                _SignalBaseInterface* s = *it;
                ++it;
                s->slot_disconnect(p);
            }
        }
    }

private:
    sender_set m_senders;
};

template <class mt_policy, typename... Args>
class SignalWithThreadPolicy : public _SignalBase<mt_policy> {
private:
    typedef _SignalBase<mt_policy> base;

protected:
    typedef typename base::connections_list connections_list;

public:
    SignalWithThreadPolicy() {}

    template <class desttype>
    void connect(desttype* pclass, void (desttype::*pmemfun)(Args...)) {
        LockBlock<mt_policy> lock(this);
        this->m_connected_slots.push_back(_OpaqueConnection(pclass, pmemfun));
        pclass->signal_connect(static_cast<_SignalBaseInterface*>(this));
    }

    void emit(Args... args) {
        LockBlock<mt_policy> lock(this);
        this->m_current_iterator = this->m_connected_slots.begin();
        while (this->m_current_iterator != this->m_connected_slots.end()) {
            _OpaqueConnection const& conn = *this->m_current_iterator;
            ++(this->m_current_iterator);
            conn.emit<Args...>(args...);
        }
    }

    void operator()(Args... args) { emit(args...); }
};

// Alias with default thread policy. Needed because both default arguments
// and variadic template arguments must go at the end of the list, so we
// can't have both at once.
template <typename... Args>
using Signal = SignalWithThreadPolicy<SIGSLOT_DEFAULT_MT_POLICY, Args...>;

// The previous verion of sigslot didn't use variadic templates, so you would
// need to write "sigslot::signal2<Arg1, Arg2>", for example.
// Now you can just write "sigslot::signal<Arg1, Arg2>", but these aliases
// exist for backwards compatibility.
template <typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
using Signal0 = SignalWithThreadPolicy<mt_policy>;

template <typename A1, typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
using Signal1 = SignalWithThreadPolicy<mt_policy, A1>;

template <typename A1,
         typename A2,
         typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
using Signal2 = SignalWithThreadPolicy<mt_policy, A1, A2>;

template <typename A1,
          typename A2,
          typename A3,
          typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
using Signal3 = SignalWithThreadPolicy<mt_policy, A1, A2, A3>;

template <typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
using Signal4 = SignalWithThreadPolicy<mt_policy, A1, A2, A3, A4>;

template <typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
using Signal5 = SignalWithThreadPolicy<mt_policy, A1, A2, A3, A4, A5>;

template <typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
using Signal6 = SignalWithThreadPolicy<mt_policy, A1, A2, A3, A4, A5, A6>;

template <typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
using Signal7 =
    SignalWithThreadPolicy<mt_policy, A1, A2, A3, A4, A5, A6, A7>;

template <typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
using Signal8 =
    SignalWithThreadPolicy<mt_policy, A1, A2, A3, A4, A5, A6, A7, A8>;

} // namespace rtcbase

#endif  //__RTCBASE_SIGSLOT_H_


