// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// There are multiple possible exit points within the editor, namely assert.h
// may exit the program after a failed assertion. To gracefully handle this and
// shut down curses, the low level routines for start/stopping curses have to be
// exported in some location that doesn't pull in a lot of other stuff. That's
// what this compilation unit is for.

#ifndef SRC_CURSES_LOW_LEVEL_H_
#define SRC_CURSES_LOW_LEVEL_H_

namespace e {
void InitializeCurses();
void EndCurses();
}

#endif  // SRC_CURSES_LOW_LEVEL_H_
