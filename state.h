// -*- C++ -*-

#include <string>
#include <vector>

#include "buffer.h"
#include "statusbar.h"

namespace e {

  class State
  {
  private:
	std::vector<Buffer *> buffers_;
	Buffer *active_buffer_;
	int window_top_line;
	std::string status_;
	StatusBar statusbar_;
	
	void draw_tab_bar();
	void draw_active_buffer();
	void draw_status();

	void update_status();

  public:
	
	State();
	State(Buffer *);
	~State();
	
	void redraw();
	void scroll_window_up();
	void scroll_window_down();
	
	int get_cursor(int &, int &);
	void loop();
	
  };
	
}
