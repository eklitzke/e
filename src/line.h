// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// Implementation of a line of code

#ifndef SRC_LINE_H_
#define SRC_LINE_H_

#include <boost/function.hpp>
#include <v8.h>

#include <string>
#include <vector>

#include "./embeddable.h"

using v8::Handle;
using v8::Value;

namespace e {

//typedef boost::function<void(const std::string&)> StringCallback;

class Line: public Embeddable {
 public:
  Line() {}
  explicit Line(const std::string &line);

  void Replace(const std::string&);
  const std::string& ToString() const;
  //void OnChange(StringCallback);
  Handle<Value> ToScript();

 public:
  std::string value;
  //private:
  //std::vector<StringCallback> callbacks_;
};
}

#endif  // SRC_LINE_H_
