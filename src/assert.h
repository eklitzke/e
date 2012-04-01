// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_ASSERT_H_
#define SRC_ASSERT_H_

namespace e {
// Halt execution and exit with an error code. Takes a format message and
// arguments exactly identical to printf.
void Panic(const char *format, ...);

// Like Panic(), but called by the ASSERT macro to log a traceback.
void PrintAssertThenExit(const char *, const char *, int);
}

// This is our custom assertion function. It varies from the standard assert(3)
// in a few ways:
//
//  * curses will be properly de-initialized before printing anything
//  * a traceback is shown after the assertion statement (the last/most recent
//    statement will be displayed LAST in the traceback)
//
// For these reasons, you should always use this ASSERT macro instead of
// assert(3). One important difference from assert(3) that you should know about
// is that assert(3) is included by default, and elided when NDEBUG is
// defined. By contrast, this macro is only compiled in when DEBUG is defined
// (NDEBUG is not checked).
#ifdef DEBUG
#define ASSERT(expr) do {                                   \
    if (static_cast<bool>(expr) == false) {                 \
      ::e::PrintAssertThenExit(#expr, __FILE__, __LINE__);  \
    }                                                       \
  } while (0)
#else   // DEBUG
#define ASSERT(e) do { (e); } while (0)
#endif  // DEBUG
#endif  // SRC_ASSERT_H_
