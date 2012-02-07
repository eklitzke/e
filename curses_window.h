// -*- C++ -*-

#ifndef _e_curses_window_h_
#define _e_curses_window_h_

#include <curses.h>

#include "window.h"

namespace e {

  class CursesWindow : public Window {
  private:
    char *c_clearscreen_;
    WINDOW *window_;

    void clear(void);
    void reset_cursor(void);
    int scr_lines(void);
    void loop_once(void);
    void update(void);
    void scr_lines_cols(int &, int &);
    void render_line(int, const std::string &);

  public:
    CursesWindow();
    CursesWindow(Buffer *);
    ~CursesWindow();

    void loop(void);
    void draw_tabs(void);
    void draw_buffer(void);
    void draw_status(void);
  };

}

#endif
