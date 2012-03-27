// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// Implementation of an array zipper.

#ifndef SRC_ZIPPER_H_
#define SRC_ZIPPER_H_

#include <string.h>  // for memcpy

#include <algorithm>
#include <vector>

#include "./assert.h"

namespace e {

template <typename T>
class Zipper {
 public:
  Zipper() {}

  // The number of elements in the zipper
  inline size_t Size() const { return front_.size() + back_.size(); }

  inline void Clear() {
    front_.clear();
    back_.clear();
  }

  // Insert an element at an arbitrary position
  inline void Insert(size_t position, T val) {
    Refocus(position);
    front_.push_back(val);
  }

  // Insert more than one element
  void Insert(size_t position, const T vals[], size_t num_elems);

  // Chop the zipper to be some new size
  void Chop(size_t new_length);

  // Append to the zipper
  inline void Append(const T vals[], size_t length) {
    Insert(Size(), vals, length);
  }

  // Erase count characters starting from position
  void Erase(size_t position, size_t count = 1);

  // Write the contents to a buffer. This buffer *must* be large enough to hold
  // the zipper contents
  void ToBuffer(T buffer[], bool refocus) const;

  T operator[](size_t offset) const;

 private:
  // Character data for the zipper; these are declared as mutable to allow
  // refocusing on certain "const" operations.
  mutable std::vector<T> front_;
  mutable std::vector<T> back_;

  void Refocus(const size_t);
  void Flatten() const;  // actually mutates front_ and back_!
};


template <typename T>
void Zipper<T>::Insert(size_t position, const T vals[], size_t num_elems) {
  Refocus(position);
  for (size_t i = 0; i < num_elems; i++) {
    front_.push_back(vals[i]);
  }
}

template <typename T>
void Zipper<T>::Chop(size_t new_length) {
  Refocus(new_length);
  back_.clear();
}

template <typename T>
void Zipper<T>::Erase(size_t position, size_t count) {
  Refocus(position + count);
  for (size_t i = 0; i < count; i++) {
    front_.pop_back();
  }
}

template <typename T>
void Zipper<T>::ToBuffer(T buffer[], bool refocus) const {
  if (refocus) {
    Flatten();
  }
  // copy the front
  if (!front_.empty()) {
    memcpy(static_cast<void *>(buffer),
           static_cast<const void *>(front_.data()),
           front_.size() * sizeof(T));
  }
  // copy the back
  if (!back_.empty()) {
    std::vector<T> back_copy = back_;
    std::reverse(back_copy.begin(), back_copy.end());
    memcpy(static_cast<void *>(buffer),
           static_cast<const void *>(back_copy.data()),
           back_copy.size() * sizeof(T));
  }
}

template <typename T>
void Zipper<T>::Refocus(const size_t position) {
  ASSERT(position <= Size());
  const size_t current = front_.size();
  if (current > position) {
    for (size_t i = 0; i < current - position; i++) {
      back_.push_back(front_.back());
      front_.pop_back();
    }
  } else if (current < position) {
    for (size_t i = 0; i < position - current; i++) {
      front_.push_back(back_.back());
      back_.pop_back();
    }
  }
  ASSERT(front_.size() == position);
}

template <typename T>
void Zipper<T>::Flatten() const {
  const size_t back_size = back_.size();
  for (size_t i = 0; i < back_size; i++) {
    front_.push_back(back_.back());
    back_.pop_back();
  }
  ASSERT(back_.empty());
}

template <typename T>
T Zipper<T>::operator[](size_t offset) const {
  ASSERT(offset < Size());
  size_t front_size = front_.size();
  if (offset < front_size) {
    return front_[offset];
  } else {
    // index from the back of back_
    return back_[back_.size() - offset + front_size - 1];
  }
}
}

#endif  // SRC_ZIPPER_H_
