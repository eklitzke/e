// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <curses.h>
#include <string.h>
#include <sys/time.h>

#include "./curses_window.h"

namespace e {

  CursesWindow::CursesWindow() {
    init();
  }

  CursesWindow::CursesWindow(Buffer *buf)
    :Window(buf) {
    init();
  }

  CursesWindow::~CursesWindow() {
    nocbreak();
    echo();
    endwin();
  }

  // TODO(eklitzke): use a delegating ctor instead
  void
  CursesWindow::init() {
    window_ = initscr();  // initialize curses

    // this look silly, but passing a string literal to tigetstr() causes a GCC
    // warning as tigetstr() isn't declared with a const arg
    static char c[6] = "clear";
    c_clearscreen_ = tigetstr(c);

    noecho();
    cbreak();
#ifdef HAS_COLORS
    if (has_colors()) {
      start_color();
    }
#endif
    clearok(window_, true);
    wtimeout(window_, 1000);

    clear();
  }


  void
  CursesWindow::loop_once() {
    draw_tabs();
    draw_buffer();
    draw_status();
    reset_cursor();
    doupdate();
  }

  void
  CursesWindow::loop() {
    loop_once();
    while (true) {
      char c = getch();
      if (c == 'q') {
        break;
      }
      loop_once();
    }
  }

  int
  CursesWindow::scr_lines(void) {
    int y, x;
    getmaxyx(window_, y, x);
    return y;
  }

  void
  CursesWindow::render_line(int position, const std::string &s) {
    mvaddnstr(position, 0, s.c_str(), s.size());
    wnoutrefresh(window_);
  }

  void
  CursesWindow::update(void) {
    doupdate();
  }

  void
  CursesWindow::clear(void) {
    putp(c_clearscreen_);
    wnoutrefresh(window_);
  }

  void
  CursesWindow::reset_cursor(void) {
    mvaddstr(1, 0, "");
  }

  void
  CursesWindow::draw_tabs(void) {
    attron(A_BOLD);
    attron(A_REVERSE);

    mvprintw(0, 0, "");
    const Buffer *active_buffer = state_.get_active_buffer();
    std::vector<Buffer *> *buffers = state_.get_buffers();
    std::vector<Buffer *>::iterator it;
    for (it = buffers->begin(); it != buffers->end(); ++it) {
      if ((*it) == active_buffer) {
        attroff(A_REVERSE);
        addstr((*it)->get_name().c_str());
        attron(A_REVERSE);
      } else {
        addstr((*it)->get_name().c_str());
      }
    }

    int width = getmaxx(window_);
    int x = getcurx(window_);
    printw("%*s", width - x, "");

    attroff(A_BOLD);
    attroff(A_REVERSE);
  }

  void
  CursesWindow::draw_buffer(void) {
    int height, width;
    getmaxyx(window_, height, width);

    const Buffer *active_buffer = state_.get_active_buffer();
    const std::vector<std::string *> *lines =
      active_buffer->get_lines(height - 3);

    for (size_t i = 0; i < lines->size(); i++) {
      std::string *s = lines->at(i);
      mvaddnstr(i + 1, 0, s->c_str(), s->size());
    }

    delete lines;
  }

  void
  CursesWindow::draw_status(void) {
    int height, width;
    getmaxyx(window_, height, width);
    attron(A_BOLD);
    attron(A_REVERSE);

    const Buffer *buf = state_.get_active_buffer();

    std::string dirty_status;
    if (state_.get_active_buffer()->is_dirty()) {
      dirty_status = "**";
    } else {
      dirty_status = "--";
    }

    const int bufwidth = 16;
    mvwprintw(window_, height - 2, 0, "--:%s-  %-*s",
              dirty_status.c_str(),
              bufwidth,
              buf->get_name().substr(0, bufwidth).c_str());

    int line, col;
    buf->cursor_pos(&line, &col);

    int maxy = getmaxy(window_);

    // the percentage is the percentage of the last line shown on the screen
    int pct;
    if (buf->num_lines() == 0) {
      pct = 100;
    } else {
      size_t last_line = buf->get_window_top() + maxy - 3;
      if (last_line >= buf->num_lines()) {
        pct = 100;
      } else {
        pct = (last_line * 100) / buf->num_lines();
      }
    }
    wprintw(window_, "%3d%% (%d,%d)", pct, line + 1, col + 1);

    // get the time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t curtime = tv.tv_sec;
    char buffer[16];
    struct tm lt;
    localtime_r(&curtime, &lt);
    strftime(buffer, sizeof(buffer), "%I:%M%p", &lt);

    size_t timelen = strlen(buffer);

    int x = getcurx(window_);
    wprintw(window_, "%*s", width - x - timelen - 1, "");
    waddstr(window_, buffer);
    waddstr(window_, " ");

    attroff(A_BOLD);
    attroff(A_REVERSE);
  }
}
