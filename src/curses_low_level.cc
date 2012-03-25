// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./curses_low_level.h"

#include <curses.h>
#include <stdlib.h>

#include "./assert.h"

namespace e {

bool is_initialized = false;

void InitializeCurses() {
  if (!is_initialized) {
    is_initialized = true;
    mousemask(ALL_MOUSE_EVENTS, nullptr);
    start_color();
    use_default_colors();  // ncurses extension!
    noecho();
    nonl();  // don't turn LF into CRLF
    raw();  // read characters one at a time, and allow Ctrl-C, Ctl-Z, etc.
    ASSERT(atexit(EndCurses) == 0);
  }
}

void EndCurses() {
  if (is_initialized) {
    is_initialized = false;
    endwin();
  }
}
}