// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef LOG_H_
#define LOG_H_

#include <string>

namespace e {
  namespace log {
    void init(const std::string &);
    void log_string(const std::string &);
  }
}

#endif  // LOG_H_
