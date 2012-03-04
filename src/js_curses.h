// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// Implementation of a line of code

#ifndef SRC_JS_CURSES_H_
#define SRC_JS_CURSES_H_

#include <curses.h>
#include <v8.h>

#include <map>
#include <string>
#include <vector>

#include "./js.h"

using v8::Handle;
using v8::Value;

namespace e {
std::map<std::string, js::JSCallback> GetCursesCallbacks();
}

#endif  // SRC_JS_CURSES_H_
