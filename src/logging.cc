// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./logging.h"

#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdarg>

#include <string>

#include "./assert.h"

namespace {
e::Logger *default_logger = nullptr;

void FinishLogging() {
  if (default_logger != nullptr) {
    delete default_logger;
  }
}
}

namespace e {


void InitLogging(const std::string &name) {
  ASSERT(default_logger == nullptr);
  default_logger = new Logger(name);
  ASSERT(atexit(FinishLogging) == 0);
}

Logger::Logger(const std::string &name, int level)
    :level_(level) {
  file_ = fopen(name.c_str(), "w");
  ASSERT(file_ != nullptr);
  Log(INFO, "opened log file \"%s\" for logging",
      name.c_str());
}

Logger::~Logger() {
  fclose(file_);
}

void Logger::SetLevel(int level) {
  level_ = level;
}

void Logger::Log(int level, const std::string &fmt, ...) const {
  va_list ap;
  va_start(ap, fmt);
  VLog(level, fmt, ap);
  va_end(ap);
}

void Logger::VLog(int level, const std::string &fmt, va_list ap) const {
  if (level < level_) {
    // N.B. the way that we do logging here means that it's not possible to
    // remove log statements for low log levels (e.g. debug) at compile time,
    // which would be possible with a macro based logging implementation. In
    // that cast, the cost of a logging call is two function calls (to LOG and
    // VLog) and the cost of calling va_start() and va_end(); the va_* methods
    // may be be simple or complex depending on the host compiler.
    //
    // The flip side to this, of course, is that logging this way allows for
    // dynamically set log levels (this could be useful to expose a more
    // fully-featured logging environment to JavaScript).
    return;
  }
  std::string format;
  timeval tv;
  gettimeofday(&tv, NULL);

  tm lt;
  localtime_r(&tv.tv_sec, &lt);

  char outstr[30];
  ASSERT(strftime(outstr, sizeof(outstr), "%H:%M:%S", &lt) != 0);
  format += outstr;

  pid_t p = getpid();
  int millis = tv.tv_usec / 1000;
  snprintf(outstr, sizeof(outstr), ".%03d %5d", millis, p);
  format += outstr;

  switch (level) {
    case DBG:
      format += "  DEBUG   ";
      break;
    case INFO:
      format += "  INFO    ";
      break;
    case SCRIPT:
      format += "  SCRIPT  ";
      break;
    case WARNING:
      format += "  WARNING ";
      break;
    case ERROR:
      format += "  ERROR   ";
      break;
    case FATAL:
      format += "  FATAL   ";
      break;
    default:
      format += "  ???     ";
      break;
  }

  format += fmt;
  format += "\n";

  ASSERT(vfprintf(file_, format.c_str(), ap) > 0);

  // Flush the log. This really just ensures that all pending writes have
  // happened, and it's good enough to ensure that if our processes crashes at
  // any future point (e.g. a SEGFAULT immediately following the log statement)
  // that the data logged will have made it to disk. This is a cheap operation
  // because it doesn't actually call fsync(2) or fdatasync(2), which are way
  // overkill for what we're trying to do.
  fflush(file_);
}

void SetDefaultLogLevel(int level) {
  if (default_logger != nullptr) {
    default_logger->SetLevel(level);
  }
}

void LOG(int level, const std::string &fmt, ...) {
  if (default_logger != nullptr) {
    va_list ap;
    va_start(ap, fmt);
    default_logger->VLog(level, fmt, ap);
    va_end(ap);
  }
}
}
