// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// Implementation of a line of code

#ifndef SRC_LINE_H_
#define SRC_LINE_H_

#if 0
#include <boost/function.hpp>
#endif
#include <v8.h>

#include <string>
#include <vector>

#include "./embeddable.h"

using v8::Handle;
using v8::Value;

namespace e {

class Line: public Embeddable {
 public:
  Line() {}
  explicit Line(const std::string &line);

  void Replace(const std::string&);
  const std::string& ToString() const;
#if 0
  void OnChange(StringCallback);
#endif
  Handle<Value> ToScript();

 public:
  std::string value;
#if 0
 private:
  std::vector<StringCallback> callbacks_;
#endif
};
}

#endif  // SRC_LINE_H_
