// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_LIST_API_H_
#define SRC_LIST_API_H_

#include <v8.h>

namespace e {
// List the JS API supported by a Context, by introspecting its global
// dictionary
void ListAPI(v8::Persistent<v8::Context>);
}

#endif  // SRC_LIST_API_H_
