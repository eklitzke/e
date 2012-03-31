// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_JS_ERRNO_H_
#define SRC_JS_ERRNO_H_

#include <v8.h>

namespace e {
namespace js_errno {
bool Build(v8::Handle<v8::Object>);
}
}

#endif  // SRC_JS_ERRNO_H_
