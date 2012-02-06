#include <assert.h>
#include <curses.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace e {
  namespace term {

	static char *console_clearscreen = NULL;
	static char *console_cursorposition = NULL;
	static WINDOW *window;
	static bool has_colors_ = false;
	
	bool initialize(void)
	{
	  window = initscr(); // initialize curses

	  console_clearscreen = tigetstr("clear");
	  console_cursorposition = tigetstr("cup");

	  noecho();
	  cbreak();
	  if (has_colors()) {
		has_colors_ = true;
		start_color();
	  }
	  clearok(window, true);
	  //scrollok(window, true);

	  clear();
	  return true;
	}

	void finish(void)
	{
	  nocbreak();
	  echo();
	  endwin();
	}

	void
	invert_colors(void)
	{
	  if (has_colors_) {
		x = 1;
	  }
	}

	void reset_colors(void)
	{
	  if (has_colors_) {
		x = 1;
	  }
	}


	int scr_lines(void)
	{
	  int y, x;
	  getmaxyx(window, y, x);
	  return y;
	}

	void scr_lines_cols(int &lines, int &cols)
	{
	  getmaxyx(window, lines, cols);
	}

	void render_line(int position, const std::string &s)
	{
	  mvaddnstr(position, 0, s.c_str(), s.size());
	  wnoutrefresh(window);
	}

	void update(void) {
	  doupdate();
	}

	void clear(void)
	{
	  putp(console_clearscreen);
	  wnoutrefresh(window);
	}
  }
}
