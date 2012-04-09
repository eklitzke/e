// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_LOGGING_H_
#define SRC_LOGGING_H_

#include <cstdio>
#include <string>

namespace e {
enum LogLevel {
  DBG     = 10,
  INFO    = 20,
  SCRIPT  = 25,
  WARNING = 30,
  ERROR   = 40,
  FATAL   = 50
};

class Logger {
 public:
  explicit Logger(const std::string &log_name, int level = INFO);
  ~Logger();
  void SetLevel(int level);
  void Log(int level, const std::string &fmt, ...) const;
  void VLog(int level, const std::string &fmt, va_list ap) const;
 private:
  FILE *file_;
  int level_;
};

void InitLogging(const std::string &log_name);
void SetDefaultLogLevel(int level);
void LOG(int level, const std::string &fmt, ...);
}

#endif  // SRC_LOGGING_H_