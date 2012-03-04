// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_LIST_ENVIRONMENT_H_
#define SRC_LIST_ENVIRONMENT_H_

#include <v8.h>

namespace e {
// List the JS environment supported by a Context, by introspecting its global
// dictionary
void ListEnvironment(v8::Persistent<v8::Context>);
}

#endif  // SRC_LIST_ENVIRONMENT_H_
