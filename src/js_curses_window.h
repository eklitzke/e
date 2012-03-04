// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// Implementation of a line of code

#ifndef SRC_JS_CURSES_WINDOW_H_
#define SRC_JS_CURSES_WINDOW_H_

#include <curses.h>
#include <v8.h>

#include <string>
#include <vector>

#include "./embeddable.h"

using v8::Handle;
using v8::Value;

namespace e {
class JSCursesWindow: public Embeddable {
 public:
  explicit JSCursesWindow(WINDOW *win);
  ~JSCursesWindow();
  Handle<Value> ToScript();
 public:
  WINDOW *window_;
};
}

#endif  // SRC_JS_CURSES_WINDOW_H_
