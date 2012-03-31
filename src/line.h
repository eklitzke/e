// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// Implementation of a line in the buffer. Lines are made up of wide characters
// (therefore they support Unicode), and are implemented using "zippers"
// (somewhat like gap buffers). Each line implicitly has a "focus" point, and
// operations like inserting or deleting characters at the focus point have
// amortized O(1) running time. Moving the focus point is O(n) (the running time
// is linear with the distance that the focus point has to move). For a
// "standard" STL implementation (vectors are up to 2x overallocated) the worst
// case storage space is 2x the length of the line.
//
// For real world use cases where a user is typing or deleting consecutive
// characters in a line, the zipper representation should be very good (about
// O(1)).

#ifndef SRC_LINE_H_
#define SRC_LINE_H_

#include <v8.h>

#include <string>
#include <vector>

#include "./zipper.h"

using v8::Local;
using v8::String;
using v8::Value;

namespace e {

class Line {
 public:
  Line() {}
  explicit Line(const std::string &line) { Replace(line); }
  inline size_t Size() const { return zipper_.Size(); }

  // Replace the contents of the line with the given ASCII string
  void Replace(const std::string&);

  // Insert a character at an arbitrary position
  inline void InsertChar(size_t position, uint16_t val) {
    zipper_.Insert(position, val);
  }

  // Insert a character at an arbitrary position
  inline void InsertChar(size_t position, char val) {
    zipper_.Insert(position, static_cast<uint16_t>(val));
  }

  // Chop the string to be some new size
  inline void Chop(size_t new_length) { zipper_.Chop(new_length); }

  // Append to the string
  inline void Append(const uint16_t buf[], size_t length) {
    zipper_.Append(buf, length);
  }

  // Erase count characters starting from position
  inline void Erase(size_t position, size_t count = 1) {
    zipper_.Erase(position, count);
  }

  // Write the contents to a V8 string.
  Local<String> ToV8String(bool refocus = true) const;

  // Write the UTF-8 contents to a std::string (slow)
  std::string ToString(bool refocus = true) const;

  Local<Value> ToScript();

 private:
  Zipper<uint16_t> zipper_;
};
}

#endif  // SRC_LINE_H_
