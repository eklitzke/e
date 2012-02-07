// -*- C++ -*-

#ifndef _e_window_h_
#define _e_window_h_

#include "buffer.h"
#include "state.h"

namespace e {
  
  class Window {
  protected:
    State state_;
  public:

    Window() {};
    Window(Buffer *);

    virtual void loop(void) = 0;
    
    virtual void draw_tabs(void) = 0;
    virtual void draw_buffer(void) = 0;
    virtual void draw_status(void) = 0;
  };

}

#endif
