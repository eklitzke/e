// -*- C++ -*-

#ifndef _e_window_h_
#define _e_window_h_

#include "state.h"

namespace e {

  class Window {
  protected:
    State state;
  public:

    Window();

    virtual void loop(void);

    virtual void draw_tabs(void);
    virtual void draw_buffer(void);
    virtual void draw_status(void);
  };

}

#endif
