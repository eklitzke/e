// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// Implementation of a line of code

#ifndef SRC_LINE_H_
#define SRC_LINE_H_

#include <boost/function.hpp>

#include <string>
#include <vector>

#include "./embeddable.h"

namespace e {

typedef boost::function<void(const std::string&)> StringCallback;

class Line: public Embeddable {
 public:
  Line() {}
  explicit Line(const std::string &line);

  void Replace(const std::string&);
  const std::string& ToString() const;
  void OnChange(StringCallback);
 private:
  std::string value_;
  std::vector<StringCallback> callbacks_;
};
}

#endif  // SRC_LINE_H_
