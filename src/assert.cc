// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./assert.h"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include <boost/lexical_cast.hpp>
#include <cxxabi.h>
#include <stdio.h>
#include <stdlib.h>

#include <glog/logging.h>
#include <glog/log_severity.h>

#include <string>
#include <vector>

#include "./curses_low_level.h"

namespace e {
namespace {
void ShowBacktrace(void) {
  unw_context_t uc;
  unw_cursor_t cursor;
  unw_word_t off;
  int unknown_count = 0;
  char funcname[256];

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);
#if 0
  printf("Traceback (last function at bottom):\n");
#endif
  std::vector<std::string> functions;
  while (unw_step(&cursor) > 0) {
    if (unw_get_proc_name(&cursor, funcname, sizeof(funcname), &off) == 0) {
      if (unknown_count) {
        functions.push_back("<" +
                            boost::lexical_cast<std::string>(unknown_count) +
                            " unknown>");
        unknown_count = 0;
      }
      int status;
      char *realname = abi::__cxa_demangle(funcname, 0, 0, &status);
      if (status == 0) {
        std::string s(realname);
        functions.push_back(s);
      }
    } else {
      unknown_count++;
    }
  }
  if (unknown_count) {
    functions.push_back("<" +
                        boost::lexical_cast<std::string>(unknown_count) +
                        " unknown>");
  }

  for (size_t it = functions.size() - 1; it > 0; it--) {
    printf("%s\n", functions[it].c_str());
    fflush(stdout);
  }
}
}


void PrintAssertThenExit(const char *exprname, const char *filename, int line) {
  EndCurses();
  printf("Assertion failed <%s:%d>: %s\n(errno is %d)\n\n",
         filename, line, exprname, errno);
  ShowBacktrace();
  google::FlushLogFiles(google::INFO);
  exit(EXIT_FAILURE);
}
}
