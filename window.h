// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef WINDOW_H_
#define WINDOW_H_

#include "./buffer.h"
#include "./state.h"

namespace e {
  class Window {
  protected:
    State *state_;
  public:
    virtual void loop(void) = 0;
    virtual void draw_tabs(void) = 0;
    virtual void draw_buffer(void) = 0;
    virtual void draw_status(void) = 0;
  };
}

#endif  // WINDOW_H_
