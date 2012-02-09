// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string>

#include "./log.h"

namespace e {
  namespace log {

    static std::string out_file = "./debug.log";

    void init(const std::string &filename) {
      out_file = filename;
      open(out_file.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0660);
    }

    void log_string(const std::string &msg) {
      std::string m(msg);
      m.push_back('\n');

      int fd = open(out_file.c_str(), O_CREAT | O_APPEND | O_WRONLY);
      size_t written = 0;
      while (written < m.size()) {
        ssize_t w = write(fd, m.c_str() + written, m.size() - written);
        if (w == -1) {
          break;
        }
        written += w;
      }
      close(fd);
    }
  }
}

