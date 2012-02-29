// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//

#include "./line.h"

#include <string>
#include <vector>

#include "./embeddable.h"

namespace e {

Line::Line(const std::string &line)
    :value_(line) {
}

void Line::Replace(const std::string &new_line) {
  value_ = new_line;
  for (StringCallback &cb : callbacks_) {
    cb(new_line);
  }
}

const std::string& Line::ToString() const {
  return value_;
}

void Line::OnChange(StringCallback cb) {
  callbacks_.push_back(cb);
}
}
