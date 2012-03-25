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

#if 0
#include <boost/function.hpp>
#endif
#include <v8.h>

#include <string>
#include <vector>

#include "./embeddable.h"
#include "./token.h"

using v8::Handle;
using v8::Value;

namespace e {

class Line: public Embeddable {
 public:
  Line() {}
  explicit Line(const std::string &line) { ResetFromString(line); }
  inline size_t Size() const { return front_.size() + back_.size(); }

  // Replace the contents of the line with the given ASCII string
  void Replace(const std::string&);

  // Insert a character at an arbitrary position
  inline void InsertChar(size_t position, uint16_t val) {
    Refocus(position);
    front_.push_back(val);
  }

  // Insert a character at an arbitrary position
  inline void InsertChar(size_t position, char val) {
    Refocus(position);
    front_.push_back(static_cast<uint16_t>(val));
  }

  // Chop the string to be some new size
  void Chop(size_t new_length);

  // Append to the string
  void Append(const uint16_t *buf, size_t length);

  // Erase count characters starting from position
  void Erase(size_t position, size_t count = 1);

  // Write the contents to a buffer. This buffer *must* be large enough to hold
  // the string contents
  void ToBuffer(uint16_t *buffer, bool refocus) const;

  // Write the contents to a V8 string.
  Local<String> ToV8String(bool refocus = true) const;

  // Write the UTF-8 contents to a std::string (slow)
  std::string ToString(bool refocus = true) const;

  Local<Value> ToScript();

  uint16_t indentation() const;
  void set_indentation(uint16_t);

 private:
  // Character data for the zipper; these are declared as mutable to allow
  // refocusing on certain "const" operations.
  mutable std::vector<uint16_t> front_;
  mutable std::vector<uint16_t> back_;

  uint16_t indentation_;
  std::vector<Token> tokens_;

  void Refocus(const size_t);
  void Flatten() const;  // actually mutates front_ and back_!
  void ResetFromString(const std::string &);
};
}

#endif  // SRC_LINE_H_
