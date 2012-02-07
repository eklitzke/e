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

  CursesWindow::CursesWindow(Buffer *b)
    :Window(b)
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
  CursesWindow::loop_once()
  {
    draw_tabs();
    draw_buffer();
    draw_status();
    reset_cursor();
    doupdate();
  }

  void
  CursesWindow::loop()
  {
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

  void
  CursesWindow::reset_cursor(void)
  {
    mvaddstr(1, 0, "");
  }
  
  void
  CursesWindow::draw_tabs(void)
  {
    attron(A_BOLD);
    attron(A_REVERSE);

    mvprintw(0, 0, "");
    const Buffer *active_buffer = state_.get_active_buffer();
    std::vector<Buffer *> *buffers = state_.get_buffers();
    std::vector<Buffer *>::iterator it;
    for (it = buffers->begin(); it != buffers->end(); it++) {
      if ((*it) == active_buffer) {
        attroff(A_REVERSE);
        addstr((*it)->get_name());
        attron(A_REVERSE);
      } else {
        addstr((*it)->get_name());
      }
    }

    int height, width;
    getmaxyx(window_, height, width);

    int y, x;
    getyx(window_, y, x);

    printw("%*s", width - x, "");

    attroff(A_BOLD);
    attroff(A_REVERSE);
  }

  void
  CursesWindow::draw_buffer(void)
  {
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
  CursesWindow::draw_status(void)
  {
    int height, width;
    getmaxyx(window_, height, width);
    attron(A_BOLD);
    attron(A_REVERSE);

    mvprintw(height - 2, 0, "%*s", width, "");
    attroff(A_BOLD);
    attroff(A_REVERSE);
    
  }

}
