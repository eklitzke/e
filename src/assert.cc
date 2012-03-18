// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include "./curses_window.h"

namespace e {
namespace {
void ShowBacktrace(void) {
  unw_context_t uc;
  unw_cursor_t cursor;
  unw_word_t ip, sp, off;
  char funcname[256];

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);
  while (unw_step(&cursor) > 0) {
    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);
    if (unw_get_proc_name(&cursor, funcname, sizeof(funcname), &off) == 0) {
      printf("ip = 0x%-12lx  sp = 0x%-12lx  %s\n",
             static_cast<uint64_t>(ip), static_cast<uint64_t>(sp), funcname);
    }
  }
}
}

void PrintAssert(const char *exprname, const char *filename, int line) {
  EndCurses();
  printf("Assertion failed <%s:%d>: %s\n\n", filename, line, exprname);
  ShowBacktrace();
  exit(1);
}
}
