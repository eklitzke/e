// -*- C++ -*-

#ifndef _e_term_h_
#define _e_term_h_

#include <string>
#include <vector>

namespace e {
  namespace term {
	bool initialize(void);
	void finish(void);

	int scr_lines(void);
	void scr_lines_cols(int &, int &);

	void update(void);

	void clear(void);
	void render_line(int, const std::string &);
  }
}

#endif
