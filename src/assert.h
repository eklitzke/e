// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_ASSERT_H_
#define SRC_ASSERT_H_

#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace e {
void PrintAssert(const char *, const char *, int);

#ifdef DEBUG
#define ASSERT(expr) do {                          \
    if (!expr) {                                   \
      e::PrintAssert(#expr, __FILE__, __LINE__);   \
    }                                              \
  } while (0)
#else
#define ASSERT(e) { }
#endif
}
#endif  // SRC_ASSERT_H_
