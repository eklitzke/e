// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./module_decl.h"

#include "./assert.h"
#include "./module.h"
#include "./js_curses.h"
#include "./js_errno.h"
#include "./js_signal.h"
#include "./js_sys.h"

namespace {
bool is_initialized = false;
}
namespace e {
void InitializeBuiltinModules() {
  ASSERT(is_initialized == false);
  is_initialized = true;
  DeclareModule("curses", &e::js_curses::Build);
  DeclareModule("errno", &e::js_errno::Build);
  DeclareModule("signal", &e::js_signal::Build);
  DeclareModule("sys", &e::js_sys::Build);
}
}
