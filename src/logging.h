// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_LOGGING_H_
#define SRC_LOGGING_H_

#include <cstdio>
#include <cstdarg>
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
  inline int GetLevel() { return level_; }
  inline void SetLevel(int level) { level_ = level; }
  void Log(int level, const std::string &fmt, ...) const;
  void VLog(int level, const std::string &fmt, va_list ap) const;
 private:
  FILE *file_;
  int level_;
};


void InitLogging(const std::string &log_name);
void SetDefaultLogLevel(int level);

// Log a message to the default logger at some log level. This method takes a
// printf style format string and arguments.
void LOG(int level, const std::string &fmt, ...);
}

#endif  // SRC_LOGGING_H_
