#include <string>

#include "state.h"
#include "term.h"

namespace e {

  State::State()
	:active_buffer_(NULL)
  {
	term::initialize();
  }

  State::State(Buffer *b)
	:active_buffer_(b)
  {
	term::initialize();
	buffers_.push_back(b);
  }

  State::~State()
  {
	term::finish();
  }

  void
  State::draw_tab_bar()
  {
	term::render_line(0, "*temp*");
  }

  void
  State::draw_active_buffer()
  {
	int lines_to_draw = term::scr_lines() - 3;

	if (active_buffer_ == NULL) {
	  for (int i = 0; i < lines_to_draw; i++) {
		term::render_line(i + 1, "~");
	  }
	} else {
	  int wt = active_buffer_->get_window_top();
	  std::vector<std::string *> *lines = active_buffer_->get_lines(wt, wt + lines_to_draw);
	  int i = 0;
	  for (; i < lines->size(); i++) {
		std::string *s = lines->at(i);
		term::render_line(i + 1, *s);
	  }
	  for (; i < lines_to_draw; i++) {
		term::render_line(i + 1, "~");
	  }
	  delete lines;
	}
	  
  }

  void
  State::draw_status()
  {
	int lines, cols;
	term::scr_lines_cols(lines, cols);

	//std::string tilde_line('-', cols);
	std::string tilde_line("========================");
	term::render_line(lines - 2, tilde_line);
	term::render_line(lines - 1, "");
  }

  void
  State::redraw()
  {
	draw_tab_bar();
	draw_active_buffer();
	draw_status();
	term::update();
  }

}

