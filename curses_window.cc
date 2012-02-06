#include <curses.h>

#include "curses_window.h"

namespace e {

  CursesWindow::CursesWindow()
  {
    window_ = initscr(); // initialize curses
    
    c_clearscreen_ = tigetstr("clear");
    
    noecho();
    cbreak();
    if (has_colors()) {
      start_color();
    }
    clearok(window_, true);
    //scrollok(window, true);
    
    clear();
  }

  CursesWindow::~CursesWindow()
  {
    nocbreak();
    echo();
    endwin();
  }

  void
  CursesWindow::loop()
  {
    draw_tabs();
    draw_buffer();
    draw_status();
    while (true) {
      getch();
      draw_tabs();
      draw_buffer();
      draw_status();
    }
  }

  int
  CursesWindow::scr_lines(void)
  {
    int y, x;
    getmaxyx(window_, y, x);
    return y;
  }
  
  void
  CursesWindow::scr_lines_cols(int &lines, int &cols)
  {
    getmaxyx(window_, lines, cols);
  }

  void
  CursesWindow::render_line(int position, const std::string &s)
  {
    mvaddnstr(position, 0, s.c_str(), s.size());
    wnoutrefresh(window_);
  }

  void
  CursesWindow::update(void) {
    doupdate();
  }

  void
  CursesWindow::clear(void)
  {
    putp(c_clearscreen_);
    wnoutrefresh(window_);
  }


}
