// (c) Copyright 2022 Aaron Kimball

#ifndef _TINY_COLLECTIONS_H_
#define _TINY_COLLECTIONS_H_

#include<stddef.h> // For size_t
#include<string.h> // For memmove()

#ifdef __AVR__
// Define std::initializer_list<T> ourselves.
#include "tc/tc_initializer_list.h"
#else // ! __AVR__
// Use g++ stdlib
#include<initializer_list>
#endif // __AVR__

#include "tc/tc_new.h"
#include "tc/tc_type_traits.h"

namespace tc {

  /////////////////       const_array<T, N>     /////////////////


  // A wrapper around a statically-defined array of const items, of known size.
  // This wrapper does not manage the data elements nor will it deallocate them when it goes out of
  // scope.
  template<typename T, size_t N> class const_array {
  public:
    constexpr const_array(const_array<T, N> &other) = default;

    // No copy operator; array components are fixed.
    const_array<T, N> &operator=(const_array<T, N> &copy_src) = delete;

    // No move operator; array components are fixed.
    const_array<T, N> &operator=(const_array<T, N> &&move_src) = delete;

    const T& operator[](size_t idx) const {
      return _data[idx];
    };

    size_t size() const { return N; }; // number of valid elements.
    size_t capacity() const { return N; }; // number of memory slots provisioned in array.
    bool empty() const { return N == 0; };

    /** Return true if you can use this subscript index safely. */
    bool in_range(size_t idx) const {
      return idx >= 0 && idx < N;
    };

    // C++ iterator interface.
    const T* begin() const {
      return _data;
    };

    const T* end() const {
      return &(_data[N]);
    };

    const T _data[N]; // array of immutable data.
  }; // class const_array<T>

  // Specialization for zero-length array.
  template<typename T> class const_array<T, 0> {
  public:
    constexpr const_array() = default;
    const_array<T, 0> &operator=(const_array<T, 0> &copy_src) = delete;
    const_array<T, 0> &operator=(const_array<T, 0> &&move_src) = delete;

    const T& operator[](size_t idx) const {
      return *((T*)NULL); // invalid operation.
    };

    size_t size() const { return 0; }; // number of valid elements.
    size_t capacity() const { return 0; }; // number of memory slots provisioned in array.
    bool empty() const { return true; };

    /** Return true if you can use this subscript index safely. */
    bool in_range(size_t idx) const {
      return false;
    };

    // C++ iterator interface.
    const T* begin() const {
      return NULL;
    };

    const T* end() const {
      return NULL;
    };
  }; // class const_array<T, 0>




  /////////////////       vector<T>     /////////////////


  constexpr size_t initial_vector_capacity = 8;

  // A mutable, resizeable array-like collection of potentially-mutable items.
  template<typename T> class vector {
  public:
    vector(): _count(0), _capacity(initial_vector_capacity) {
      _data = reinterpret_cast<T*>(new unsigned char[sizeof(T) * _capacity]);
    };

    vector(size_t initial_cap): _count(0), _capacity(initial_cap) {
      if (_capacity == 0) {
        _capacity = 1;
      }
      _data = reinterpret_cast<T*>(new unsigned char[sizeof(T) * _capacity]);
    };

    /** Copy constructor. */
    vector(const vector<T> &other): _count(other._count), _capacity(other._capacity) {
      _data = reinterpret_cast<T*>(new unsigned char[sizeof(T) * _capacity]);
      // Initialize elements via placement-new copy constructor.
      for (size_t i = 0; i < _count; i++) {
        new (&(_data[i])) T(other._data[i]);
      }
    };

    /** Copy constructor from explicit array. */
    template<size_t N> vector(const T (&arr)[N]): _count(N) {
      if (N > initial_vector_capacity) {
        _capacity = N;
      } else {
        _capacity = initial_vector_capacity;
      }

      _data = reinterpret_cast<T*>(new unsigned char[sizeof(T) * _capacity]);
      for (size_t i = 0; i < _count; i++) {
        new (&(_data[i])) T(arr[i]);
      }
    };

    /** Copy constructor from const_array<T>. */
    template<size_t N>
    vector(const const_array<T, N> &arr): _count(arr.size()), _capacity(arr.size()) {
      _data = reinterpret_cast<T*>(new unsigned char[sizeof(T) * _capacity]);
      // Initialize elements via placement-new copy constructor.
      for (size_t i = 0; i < _count; i++) {
        new (&(_data[i])) T(arr[i]);
      }
    }

    /** Copy constructor from brace-initialized list. */
    vector(std::initializer_list<T> lst): _count(lst.size()) {
      if (lst.size() > initial_vector_capacity) {
        _capacity = lst.size();
      } else {
        _capacity = initial_vector_capacity;
      }

      _data = reinterpret_cast<T*>(new unsigned char[sizeof(T) * _capacity]);
      size_t i = 0;
      for (const T* it=lst.begin(); it != lst.end(); it++) {
        new (&(_data[i++])) T(*it);
      }
    }

    /** Move constructor. */
    vector(vector<T> &&other) noexcept: _count(other._count), _capacity(other._capacity),
        _data(other._data) {

      other._data = NULL;
      other._count = 0;
      other._capacity = 0;
    };

    ~vector() {
      _deallocate();
    };

    // Move operator; assume move_src's data.
    vector<T> &operator=(vector<T> &&move_src) noexcept {
      _deallocate(); // free existing _data (if any) first.

      _data = move_src._data;
      _count = move_src._count;
      _capacity = move_src._capacity;

      move_src._data = NULL;
      move_src._count = 0;
      move_src._capacity = 0;

      return *this;
    };

    // Copy operator: duplicate the elements of another vector<T>.
    vector<T> &operator=(const vector<T> &copy_src) {
      _deallocate(); // free existing _data (if any) first.

      _count = copy_src._count;
      _capacity = copy_src._capacity;
      _data = reinterpret_cast<T*>(new unsigned char[sizeof(T) * _capacity]);
      // Initialize elements via placement-new copy constructor.
      for (size_t i = 0; i < _count; i++) {
        new (&(_data[i])) T(copy_src._data[i]);
      }

      return *this;
    };

    // Copy operator: duplicate the elements of a const_array<T>
    template<size_t N>
    vector<T> &operator=(const const_array<T, N> &copy_src) {
      _deallocate(); // free existing _data (if any) first.

      _count = copy_src.size();
      _capacity = _count;
      _data = reinterpret_cast<T*>(new unsigned char[sizeof(T) * _capacity]);
      // Initialize elements via placement-new copy constructor.
      for (size_t i = 0; i < _count; i++) {
        new (&(_data[i])) T(copy_src[i]);
      }

      return *this;
    };

    const T& operator[](size_t idx) const {
      return _data[idx];
    };

    T& operator[](size_t idx) {
      return _data[idx];
    };

    size_t size() const { return _count; }; // number of valid elements.
    size_t capacity() const { return _capacity; }; // number of memory slots provisioned in array.
    bool empty() const { return _count == 0; };
    void reserve(size_t minCapacity) { _ensure_capacity(minCapacity - 1); };

    void clear() {
      if constexpr(!tc_is_scalar<T>::value()) {
        for (size_t i = 0; i < _count; i++) {
          // Explicitly call destructor for each item one-by-one.
          _data[i].~T();
        }
      }

      // Set count=0 on the way out.
      _count = 0;
    }

    /** Return true if you can use this subscript index safely. */
    bool in_range(size_t idx) const {
      return idx >= 0 && idx < _count;
    };

    // Append item to the vector, allocating more space if necessary. Returns the
    // index where the item was stored.
    size_t push_back(const T& item) {
      _ensure_capacity(_count);
      // Use placement-new to initialize a T at the next array index by copy-constructing from `item`.
      new (&(_data[_count])) (T)(item);
      return _count++;
    };

    // Insert item at position `pos`, moving `pos`..`_count-1` down by 1.
    size_t insert_at(const T& item, size_t pos) {
      _ensure_capacity(_count);

      if (pos == _count) {
        // Same as push_back().
        new (&(_data[_count++])) (T)(item);
      } else {
        // Move everything down by 1 element to make room
        size_t move_bytes = reinterpret_cast<const unsigned char*>(end()) -
            reinterpret_cast<const unsigned char*>(&(_data[pos]));

        memmove(reinterpret_cast<unsigned char*>(&(_data[pos + 1])),
            reinterpret_cast<const unsigned char*>(&(_data[pos])),
            move_bytes);

        // And then do the insert.
        new (&(_data[pos])) (T)(item);
        _count++;
      }

      return pos;
    };

    // C++ iterator interface.
    const T* begin() const {
      return &(_data[0]);
    };

    T* begin() {
      return &(_data[0]);
    };

    const T* end() const {
      return &(_data[_count]);
    };

    // Erase the value at the iterator location.
    void erase(T* atIter) {
      T* last = &(_data[_count - 1]); // The last valid element of the array.
      T* end = &(_data[_count]); // 1 T after the last valid element of the array.
      T* next = reinterpret_cast<T*>(reinterpret_cast<unsigned char*>(atIter) + sizeof(T));

      if (atIter < last) {
        // Move subsequent elements up by one position.
        memmove((unsigned char*)atIter, (unsigned char*)next,
            (unsigned char*)end - (unsigned char*)next);
      }

      _count--;
    }

  private:
    // Ensures that `maxIdx` is a safe index into _data, resizing to grow if necessary.
    void _ensure_capacity(size_t maxIdx) {
      while (maxIdx >= _capacity) {
        _capacity *= 2;
      }

      T* newData = reinterpret_cast<T*>(new unsigned char[sizeof(T) * _capacity]);
      for (size_t i = 0; i < _count; i++) {
        // Initialize `count` new T elements that move-construct from the old array.
        new (&(newData[i])) (T)(static_cast<T&&>(_data[i]));
      }

      delete [] reinterpret_cast<unsigned char*>(_data);
      _data = newData;
    };

    /** Called in destructor and other places where elements must be deconstructed. */
    void _deallocate() {
      if (NULL == _data) {
        return; // Nothing to do.
      }

      if constexpr(!tc_is_scalar<T>::value()) {
        for (size_t i = 0; i < _count; i++) {
          // Explicitly call destructor for each item one-by-one.
          _data[i].~T();
        }
      }

      // Free memory without triggering object destructors during delete[].
      delete[] reinterpret_cast<unsigned char*>(_data);
      _data = NULL;
    }

    size_t _count; // number of filled slots.
    size_t _capacity; // number of slots available.
    T* _data;
  }; // class vector<T>

}; // namespace tc


#endif // _TINY_COLLECTIONS_H_
