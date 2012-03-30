// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_JS_SYS_H_
#define SRC_JS_SYS_H_

#include <v8.h>

namespace e {
v8::Handle<v8::ObjectTemplate> GetSysTemplate();
}

#endif  // SRC_JS_SYS_H_
