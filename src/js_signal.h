// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_JS_SIGNAL_H_
#define SRC_JS_SIGNAL_H_

#include <v8.h>

namespace e {
v8::Handle<v8::ObjectTemplate> GetSignalTemplate();
}

#endif  // SRC_JS_SIGNAL_H
