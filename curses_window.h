// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef CURSES_WINDOW_H_
#define CURSES_WINDOW_H_

#include <curses.h>
#include <string>

#include "./window.h"

namespace e {

  class CursesWindow : public Window {
  private:
    State *state_;
    char *c_clearscreen_;
    WINDOW *window_;

    void init(void);
    void clear(void);
    void reset_cursor(void);
    int scr_lines(void);
    void loop_once(void);
    void update(void);
    void render_line(int, const std::string &);

  public:
    explicit CursesWindow(State *state);
    ~CursesWindow();

    void loop(void);
    void draw_tabs(void);
    void draw_buffer(void);
    void draw_status(void);
  };
}

#endif  // CURSES_WINDOW_H_
