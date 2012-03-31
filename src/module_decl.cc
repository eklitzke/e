// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./module_decl.h"

#include "./module.h"
#include "./js_errno.h"
#include "./js_signal.h"
#include "./js_sys.h"

namespace e {
void InitializeModules() {
  DeclareModule("errno", &e::js_errno::Build);
  DeclareModule("signal", &e::js_signal::Build);
  DeclareModule("sys", &e::js_sys::Build);
}
}
