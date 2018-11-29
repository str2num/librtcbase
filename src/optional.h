/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file optional.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_OPTIONAL_H_
#define  __RTCBASE_OPTIONAL_H_

#include <algorithm>
#include <memory>
#include <utility>

#include "array_view.h"
#include "sanitizer.h"

namespace rtcbase {

namespace optional_internal {

#if RTC_HAS_ASAN

// This is a non-inlined function. The optimizer can't see inside it.  It
// prevents the compiler from generating optimized code that reads value_ even
// if it is unset. Although safe, this causes memory sanitizers to complain.
void* FunctionThatDoesNothingImpl(void*);

template <typename T>
inline T* FunctionThatDoesNothing(T* x) {
    return reinterpret_cast<T*>(
            FunctionThatDoesNothingImpl(reinterpret_cast<void*>(x)));
}

#else

template <typename T>
inline T* FunctionThatDoesNothing(T* x) {
    return x;
}

#endif

struct NulloptArg;

}  // namespace optional_internal

// nullopt_t must be a non-aggregate literal type with a constexpr constructor
// that takes some implementation-defined literal type. It mustn't have a
// default constructor nor an initializer-list constructor.
// See:
// http://en.cppreference.com/w/cpp/utility/optional/nullopt_t
// That page uses int, though this seems to confuse older versions of GCC.
struct nullopt_t {
    constexpr explicit nullopt_t(rtcbase::optional_internal::NulloptArg&) {}
};

// Specification:
// http://en.cppreference.com/w/cpp/utility/optional/nullopt
extern const nullopt_t nullopt;

// Simple std::optional-wannabe. It either contains a T or not.
//
// A moved-from Optional<T> may only be destroyed, and assigned to if T allows
// being assigned to after having been moved from. Specifically, you may not
// assume that it just doesn't contain a value anymore.
//
// Examples of good places to use Optional:
//
// - As a class or struct member, when the member doesn't always have a value:
//     struct Prisoner {
//       std::string name;
//       Optional<int> cell_number;  // Empty if not currently incarcerated.
//     };
//
// - As a return value for functions that may fail to return a value on all
//   allowed inputs. For example, a function that searches an array might
//   return an Optional<size_t> (the index where it found the element, or
//   nothing if it didn't find it); and a function that parses numbers might
//   return Optional<double> (the parsed number, or nothing if parsing failed).
//
// Examples of bad places to use Optional:
//
// - As a return value for functions that may fail because of disallowed
//   inputs. For example, a string length function should not return
//   Optional<size_t> so that it can return nothing in case the caller passed
//   it a null pointer; the function should probably use RTC_[D]CHECK instead,
//   and return plain size_t.
//
// - As a return value for functions that may fail to return a value on all
//   allowed inputs, but need to tell the caller what went wrong. Returning
//   Optional<double> when parsing a single number as in the example above
//   might make sense, but any larger parse job is probably going to need to
//   tell the caller what the problem was, not just that there was one.
//
// TODO(kwiberg): Get rid of this class when the standard library has
// std::optional (and we're allowed to use it).
template <typename T>
class Optional final {
public:
    // Construct an empty Optional.
    Optional() : _has_value(false), _empty('\0') { poison_value(); }

    Optional(rtcbase::nullopt_t)  // NOLINT(runtime/explicit)
        : Optional() {}     

    // Construct an Optional that contains a value.
    Optional(const T& value) : _has_value(true) {
        new (&_value) T(value);
    }
    
    Optional(T&& value) : _has_value(true) {
        new (&_value) T(std::move(value));
    }

    // Copy constructor: copies the value from m if it has one.
    Optional(const Optional& m) : _has_value(m._has_value) {
        if (_has_value) {
            new (&_value) T(m._value);
        } else {
            poison_value();
        }
    }

    // Move constructor: if m has a value, moves the value from m, leaving m
    // still in a state where it has a value, but a moved-from one (the
    // properties of which depends on T; the only general guarantee is that we
    // can destroy m).
    Optional(Optional&& m) : _has_value(m._has_value) {
        if (_has_value) {
            new (&_value) T(std::move(m._value));
        } else {
            poison_value();
        }
    }

    ~Optional() {
        if (_has_value) {
            _value.~T();
        } else {
            unpoison_value();
        }
    }
    
    Optional& operator=(rtcbase::nullopt_t) {
        reset();
        return *this;
    }

    // Copy assignment. Uses T's copy assignment if both sides have a value, T's
    // copy constructor if only the right-hand side has a value.
    Optional& operator=(const Optional& m) {
        if (m._has_value) {
            if (_has_value) {
                _value = m._value;  // T's copy assignment.
            } else {
                unpoison_value();
                new (&_value) T(m._value);  // T's copy constructor.
                _has_value = true;
            }
        } else {
            reset();
        }
        return *this;
    }

    // Move assignment. Uses T's move assignment if both sides have a value, T's
    // move constructor if only the right-hand side has a value. The state of m
    // after it's been moved from is as for the move constructor.
    Optional& operator=(Optional&& m) {
        if (m._has_value) {
            if (_has_value) {
                _value = std::move(m._value);  // T's move assignment.
            } else {
                unpoison_value();
                new (&_value) T(std::move(m._value));  // T's move constructor.
                _has_value = true;
            }
        } else {
            reset();
        }
        return *this;
    }

    // Swap the values if both m1 and m2 have values; move the value if only one
    // of them has one.
    friend void swap(Optional& m1, Optional& m2) {
        if (m1._has_value) {
            if (m2._has_value) {
                // Both have values: swap.
                using std::swap;
                swap(m1._value, m2._value);
            } else {
                // Only m1 has a value: move it to m2.
                m2.unpoison_value();
                new (&m2._value) T(std::move(m1._value));
                m1._value.~T();  // Destroy the moved-from value.
                m1._has_value = false;
                m2._has_value = true;
                m1.poison_value();
            }
        } else if (m2._has_value) {
            // Only m2 has a value: move it to m1.
            m1.unpoison_value();
            new (&m1._value) T(std::move(m2._value));
            m2._value.~T();  // Destroy the moved-from value.
            m1._has_value = true;
            m2._has_value = false;
            m2.poison_value();
        }
    }
    
    // Destroy any contained value. Has no effect if we have no value.
    void reset() {
        if (!_has_value) {
            return;
        }
        _value.~T();
        _has_value = false;
        poison_value();
    }

    template <class... Args>
    void emplace(Args&&... args) {
        if (_has_value) {
            _value.~T();
        } else {
            unpoison_value();
        }
        new (&_value) T(std::forward<Args>(args)...);
        _has_value = true;
    } 

    // Conversion to bool to test if we have a value.
    explicit operator bool() const { return _has_value; }
    bool has_value() const { return _has_value; }

    // Dereferencing. Only allowed if we have a value.
    const T* operator->() const {
        if (!_has_value) {
            return NULL;
        }
        return &_value;
    }
    
    T* operator->() {
        if (!_has_value) {
            return NULL;
        }
        return &_value;
    }
    
    const T& operator*() const {
        return _value;
    }
    
    T& operator*() {
        return _value;
    }
    
    const T& value() const {
        return _value;
    }
    
    T& value() {
        return _value;
    }

    // Dereference with a default value in case we don't have a value.
    const T& value_or(const T& default_val) const {
        // The no-op call prevents the compiler from generating optimized code that
        // reads value_ even if !has_value_, but only if FunctionThatDoesNothing is
        // not completely inlined; see its declaration.).
        return _has_value ? *optional_internal::FunctionThatDoesNothing(&_value)
            : default_val; 
    }
    
    // Dereference and move value.
    T move_value() {
        return std::move(_value);
    }

    // Equality tests. Two Optionals are equal if they contain equivalent values,
    // or
    // if they're both empty.
    friend bool operator==(const Optional& m1, const Optional& m2) {
        return m1._has_value && m2._has_value ? m1._value == m2._value
            : m1._has_value == m2._has_value;
    }

    friend bool operator==(const Optional& opt, const T& value) {
        return opt._has_value && opt._value == value;
    }
    
    friend bool operator==(const T& value, const Optional& opt) {
        return opt._has_value && value == opt._value;
    }

    friend bool operator==(const Optional& opt, rtcbase::nullopt_t) {
        return !opt._has_value;
    }

    friend bool operator==(rtcbase::nullopt_t, const Optional& opt) {
        return !opt._has_value;
    }     

    friend bool operator!=(const Optional& m1, const Optional& m2) {
        return m1._has_value && m2._has_value ? m1._value != m2._value
            : m1._has_value != m2._has_value;
    }

    friend bool operator!=(const Optional& opt, const T& value) {
        return !opt._has_value || opt._value != value;
    }
    
    friend bool operator!=(const T& value, const Optional& opt) {
        return !opt._has_value || value != opt._value;
    }

    friend bool operator!=(const Optional& opt, rtcbase::nullopt_t) {
        return opt._has_value;
    }

    friend bool operator!=(rtcbase::nullopt_t, const Optional& opt) {
        return opt._has_value;
    }     

private:
    // Tell sanitizers that _value shouldn't be touched.
    void poison_value() {
        rtcbase::AsanPoison(rtcbase::make_array_view(&_value, 1));
        rtcbase::MsanMarkUninitialized(rtcbase::make_array_view(&_value, 1));
    }

    // Tell sanitizers that value_ is OK to touch again.
    void unpoison_value() { rtcbase::AsanUnpoison(rtcbase::make_array_view(&_value, 1)); }

private:
    bool _has_value;  // True iff value_ contains a live value.
    union {
        // empty_ exists only to make it possible to initialize the union, even when
        // it doesn't contain any data. If the union goes uninitialized, it may
        // trigger compiler warnings.
        char _empty;
        // By placing value_ in a union, we get to manage its construction and
        // destruction manually: the Optional constructors won't automatically
        // construct it, and the Optional destructor won't automatically destroy
        // it. Basically, this just allocates a properly sized and aligned block of
        // memory in which we can manually put a T with placement new.
        T _value;
    };
};

}  // namespace rtcbase

#endif  //__RTCBASE_OPTIONAL_H_


