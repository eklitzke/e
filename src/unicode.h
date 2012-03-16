// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_UNICODE_H_
#define SRC_UNICODE_H_

#include <v8.h>
#include <unicode/unistr.h>

namespace e {
// Converts a UnicodeString object to a v8 String.
v8::Local<v8::String> UnicodeToString(const UnicodeString &str);
}

#endif  // SRC_UNICODE_H_
