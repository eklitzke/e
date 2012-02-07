// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./buffer.h"
#include "./window.h"

namespace e {

  Window::Window(Buffer *buf)
    :state_(buf) {
  }

  Window::~Window() {
  }
}
