/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.  
 */ 
 
/**
 * @file buffer.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_BUFFER_H_
#define  __RTCBASE_BUFFER_H_

#include <algorithm>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#include "memcheck.h"
#include "array_view.h"

namespace rtcbase {

namespace internal {

// (Internal; please don't use outside this file.) Determines if elements of
// type U are compatible with a BufferT<T>. For most types, we just ignore
// top-level const and forbid top-level volatile and require T and U to be
// otherwise equal, but all byte-sized integers (notably char, int8_t, and
// uint8_t) are compatible with each other. (Note: We aim to get rid of this
// behavior, and treat all types the same.)
template <typename T, typename U>
struct BufferCompat {
    static constexpr bool value =
        !std::is_volatile<U>::value &&
        ((std::is_integral<T>::value && sizeof(T) == 1)
         ? (std::is_integral<U>::value && sizeof(U) == 1)
         : (std::is_same<T, typename std::remove_const<U>::type>::value));
};

}  // namespace internal

// Basic buffer class, can be grown and shrunk dynamically.
// Unlike std::string/vector, does not initialize data when increasing size.
template <typename T>
class BufferT : public MemCheck {
    // We want T's destructor and default constructor to be trivial, i.e. perform
    // no action, so that we don't have to touch the memory we allocate and
    // deallocate. And we want T to be trivially copyable, so that we can copy T
    // instances with std::memcpy. This is precisely the definition of a trivial
    // type.
    static_assert(std::is_trivial<T>::value, "T must be a trivial type.");

    // This class relies heavily on being able to mutate its data.
    static_assert(!std::is_const<T>::value, "T may not be const");

public:
    // An empty BufferT.
    BufferT() : MemCheck("BufferT"), _size(0), _capacity(0), _data(nullptr) {
        //RTC_DCHECK(IsConsistent());
    }

    // Disable copy construction and copy assignment, since copying a buffer is
    // expensive enough that we want to force the user to be explicit about it.
    BufferT(const BufferT&) = delete;
    BufferT& operator=(const BufferT&) = delete;

    BufferT(BufferT&& buf)
        : MemCheck("BufferT"), _size(buf.size()),
        _capacity(buf.capacity()),
        _data(std::move(buf._data)) 
    {
        //RTC_DCHECK(IsConsistent());
        buf.on_moved_from();
    }

    // Construct a buffer with the specified number of uninitialized elements.
    explicit BufferT(size_t size) : BufferT(size, size) {}

    BufferT(size_t size, size_t capacity)
        : MemCheck("BufferT"), _size(size),
        _capacity(std::max(size, capacity)),
        _data(new T[_capacity]) 
    {
        //RTC_DCHECK(IsConsistent());
    }

    // Construct a buffer and copy the specified number of elements into it.
    template <typename U,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 BufferT(const U* data, size_t size) : BufferT(data, size, size) {}

    template <typename U,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 BufferT(U* data, size_t size, size_t capacity) : BufferT(size, capacity) {
                     static_assert(sizeof(T) == sizeof(U), "");
                     std::memcpy(_data.get(), data, size * sizeof(U));
                 }

    // Construct a buffer from the contents of an array.
    template <typename U,
             size_t N,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 BufferT(U (&array)[N]) : BufferT(array, N) {}

    // Get a pointer to the data. Just .data() will give you a (const) T*, but if
    // T is a byte-sized integer, you may also use .data<U>() for any other
    // byte-sized integer U.
    template <typename U = T,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 const U* data() const {
                     //RTC_DCHECK(IsConsistent());
                     return reinterpret_cast<U*>(_data.get());
                 }

    template <typename U = T,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 U* data() {
                     //RTC_DCHECK(IsConsistent());
                     return reinterpret_cast<U*>(_data.get());
                 }
    
    bool empty() const {
        return _size == 0;
    }

    size_t size() const {
        //RTC_DCHECK(IsConsistent());
        return _size;
    }

    size_t capacity() const {
        //RTC_DCHECK(IsConsistent());
        return _capacity;
    }

    BufferT& operator=(BufferT&& buf) {
        //RTC_DCHECK(IsConsistent());
        //RTC_DCHECK(buf.IsConsistent());
        _size = buf._size;
        _capacity = buf._capacity;
        _data = std::move(buf._data);
        buf.on_moved_from();
        return *this;
    }

    bool operator==(const BufferT& buf) const {
        //RTC_DCHECK(IsConsistent());
        if (_size != buf._size) {
            return false;
        }
        if (std::is_integral<T>::value) {
            // Optimization.
            return std::memcmp(_data.get(), buf._data.get(), _size * sizeof(T)) == 0;
        }
        for (size_t i = 0; i < _size; ++i) {
            if (_data[i] != buf._data[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const BufferT& buf) const { return !(*this == buf); }

    T& operator[](size_t index) {
        //RTC_DCHECK_LT(index, size_);
        return data()[index];
    }

    T operator[](size_t index) const {
        //RTC_DCHECK_LT(index, size_);
        return data()[index];
    }

    // The SetData functions replace the contents of the buffer. They accept the
    // same input types as the constructors.
    template <typename U,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 void set_data(const U* data, size_t size) {
                     //RTC_DCHECK(IsConsistent());
                     _size = 0;
                     append_data(data, size);
                 }

    template <typename U,
             size_t N,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 void set_data(const U (&array)[N]) {
                     set_data(array, N);
                 }

    void set_data(const BufferT& buf) { set_data(buf.data(), buf.size()); }

    // Replace the data in the buffer with at most |max_elements| of data, using
    // the function |setter|, which should have the following signature:
    //   size_t setter(ArrayView<U> view)
    // |setter| is given an appropriately typed ArrayView of the area in which to
    // write the data (i.e. starting at the beginning of the buffer) and should
    // return the number of elements actually written. This number must be <=
    // |max_elements|.
    template <typename U = T,
             typename F,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 size_t set_data(size_t max_elements, F&& setter) {
                     //RTC_DCHECK(IsConsistent());
                     _size = 0;
                     return append_data<U>(max_elements, std::forward<F>(setter));
                 }

    // The AppendData functions add data to the end of the buffer. They accept
    // the same input types as the constructors.
    template <typename U,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 void append_data(const U* data, size_t size) {
                     //RTC_DCHECK(IsConsistent());
                     const size_t new_size = _size + size;
                     ensure_capacity_with_headroom(new_size, true);
                     static_assert(sizeof(T) == sizeof(U), "");
                     std::memcpy(_data.get() + _size, data, size * sizeof(U));
                     _size = new_size;
                     //RTC_DCHECK(IsConsistent());
                 }

    template <typename U,
             size_t N,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 void append_data(const U (&array)[N]) {
                     append_data(array, N);
                 }

    void append_data(const BufferT& buf) { append_data(buf.data(), buf.size()); }

    template <typename U,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 void append_data(const U& item) {
                     append_data(&item, 1);
                 }

    // Append at most |max_elements| to the end of the buffer, using the function
    // |setter|, which should have the following signature:
    //   size_t setter(ArrayView<U> view)
    // |setter| is given an appropriately typed ArrayView of the area in which to
    // write the data (i.e. starting at the former end of the buffer) and should
    // return the number of elements actually written. This number must be <=
    // |max_elements|.
    template <typename U = T,
             typename F,
             typename std::enable_if<
                 internal::BufferCompat<T, U>::value>::type* = nullptr>
                 size_t append_data(size_t max_elements, F&& setter) {
                     //RTC_DCHECK(IsConsistent());
                     const size_t old_size = _size;
                     set_size(old_size + max_elements);
                     U* base_ptr = data<U>() + old_size;
                     size_t written_elements = setter(rtcbase::ArrayView<U>(base_ptr, max_elements));

                     //RTC_CHECK_LE(written_elements, max_elements);
                     _size = old_size + written_elements;
                     //RTC_DCHECK(IsConsistent());
                     return written_elements;
                 }

    // Sets the size of the buffer. If the new size is smaller than the old, the
    // buffer contents will be kept but truncated; if the new size is greater,
    // the existing contents will be kept and the new space will be
    // uninitialized.
    void set_size(size_t size) {
        ensure_capacity_with_headroom(size, true);
        _size = size;
    }

    // Ensure that the buffer size can be increased to at least capacity without
    // further reallocation. (Of course, this operation might need to reallocate
    // the buffer.)
    void ensure_capacity(size_t capacity) {
        // Don't allocate extra headroom, since the user is asking for a specific
        // capacity.
        ensure_capacity_with_headroom(capacity, false);
    }

    // Resets the buffer to zero size without altering capacity. Works even if the
    // buffer has been moved from.
    void clear() {
        _size = 0;
        //RTC_DCHECK(IsConsistent());
    }

    // Swaps two buffers. Also works for buffers that have been moved from.
    friend void swap(BufferT& a, BufferT& b) {
        using std::swap;
        swap(a._size, b._size);
        swap(a._capacity, b._capacity);
        swap(a._data, b._data);
    }

private:
    void ensure_capacity_with_headroom(size_t capacity, bool extra_headroom) {
        //RTC_DCHECK(IsConsistent());
        if (capacity <= _capacity) {
            return;
        }

        // If the caller asks for extra headroom, ensure that the new capacity is
        // >= 1.5 times the old capacity. Any constant > 1 is sufficient to prevent
        // quadratic behavior; as to why we pick 1.5 in particular, see
        // https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md and
        // http://www.gahcep.com/cpp-internals-stl-vector-part-1/.
        const size_t new_capacity =
            extra_headroom ? std::max(capacity, _capacity + _capacity / 2)
            : capacity;

        std::unique_ptr<T[]> new_data(new T[new_capacity]);
        std::memcpy(new_data.get(), _data.get(), _size * sizeof(T));
        _data = std::move(new_data);
        _capacity = new_capacity;
        //RTC_DCHECK(IsConsistent());
    }

    // Precondition for all methods except Clear and the destructor.
    // Postcondition for all methods except move construction and move
    // assignment, which leave the moved-from object in a possibly inconsistent
    // state.
    bool is_consistent() const {
        return (_data || _capacity == 0) && _capacity >= _size;
    }

    // Called when *this has been moved from. Conceptually it's a no-op, but we
    // can mutate the state slightly to help subsequent sanity checks catch bugs.
    void on_moved_from() {
#ifdef NDEBUG
        // Make *this consistent and empty. Shouldn't be necessary, but better safe
        // than sorry.
        _size = 0;
        _capacity = 0;
#else
        // Ensure that *this is always inconsistent, to provoke bugs.
        _size = 1;
        _capacity = 0;
#endif
    }

    size_t _size;
    size_t _capacity;
    std::unique_ptr<T[]> _data;
};

// By far the most common sort of buffer.
using Buffer = BufferT<uint8_t>;

}  // namespace rtcbase

#endif  //__RTCBASE_BUFFER_H_


