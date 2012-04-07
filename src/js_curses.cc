// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <curses.h>
#include <v8.h>
#include <glog/logging.h>
#include <glog/log_severity.h>

#include <string>

#include "./js.h"
#include "./js_curses_window.h"
#include "./module.h"

using v8::AccessorInfo;
using v8::Arguments;
using v8::Boolean;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Integer;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Undefined;
using v8::Value;

#define CURSES_VOID_FUNC(capname, func)\
  Handle<Value> Curses ## capname(const Arguments& args) {  \
    HandleScope scope;                                      \
    Local<Integer> ret = Integer::New(func());              \
    return scope.Close(ret);                                \
  }

#define CURSES_WINDOW_FUNC(capname, func)                   \
  Handle<Value> Curses ## capname(const Arguments& args) {  \
    HandleScope scope;                                      \
    Local<Integer> ret = Integer::New(func(stdscr));        \
    return scope.Close(ret);                                \
  }

#define CURSES_ACCESSOR(macroname)                                      \
  Handle<Value> JS_##macroname(Local<String> property,                  \
                               const AccessorInfo& info) {              \
    HandleScope scope;                                                  \
    return scope.Close(Integer::New(macroname));                        \
  }


#define NEW_INTEGER(obj, name) AddInteger(obj, #name, name)
#define DECLARE_ACCESSOR(obj, macroname)                                \
  AddAccessor(obj, #macroname, JS_##macroname, nullptr)

namespace {
// @class: curses
// @description: A low-level interface to ncurses.

// @accessor: COLOR_PAIRS
// @description: The number of color pairs the terminal supports.
CURSES_ACCESSOR(COLOR_PAIRS)

// @accessor: COLORS
// @description: The number of colors the terminal supports.
CURSES_ACCESSOR(COLORS)

// @accessor: COLS
// @description: The width of the screen (i.e. the number of columns).
CURSES_ACCESSOR(COLS)

// @accessor: ESCDELAY
// @description: The number of milliseconds ncurses will wait after reading an
//               escape character
//
// XXX: This is a non-portable ncurses extension. The "portable" way to do this
// is to simply read the ESCDELAY global variable. This has the advantage of
// being threadsafe, however.
Handle<Value> JS_ESCDELAY(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  return scope.Close(Integer::New(get_escdelay()));
}

// @accessor: LINES
// @description: The height of the screen (i.e. the number of lines).
CURSES_ACCESSOR(LINES)

// @accessor: TABSIZE
// @description: The number of columns in a tab character.
CURSES_ACCESSOR(TABSIZE)

// @accessor: COLOR_BLACK
CURSES_ACCESSOR(COLOR_BLACK)

// @accessor: COLOR_RED
CURSES_ACCESSOR(COLOR_RED)

// @accessor: COLOR_GREEN
CURSES_ACCESSOR(COLOR_GREEN)

// @accessor: COLOR_YELLOW
CURSES_ACCESSOR(COLOR_YELLOW)

// @accessor: COLOR_BLUE
CURSES_ACCESSOR(COLOR_BLUE)

// @accessor: COLOR_MAGENTA
CURSES_ACCESSOR(COLOR_MAGENTA)

// @accessor: COLOR_CYAN
CURSES_ACCESSOR(COLOR_CYAN)

// @accessor: COLOR_WHITE
CURSES_ACCESSOR(COLOR_WHITE)

// @accessor: A_NORMAL
CURSES_ACCESSOR(A_NORMAL)

// @accessor: A_STANDOUT
CURSES_ACCESSOR(A_STANDOUT)

// @accessor: A_UNDERLINE
CURSES_ACCESSOR(A_UNDERLINE)

// @accessor: A_REVERSE
CURSES_ACCESSOR(A_REVERSE)

// @accessor: A_BLINK
CURSES_ACCESSOR(A_BLINK)

// @accessor: A_DIM
CURSES_ACCESSOR(A_DIM)

// @accessor: A_BOLD
CURSES_ACCESSOR(A_BOLD)

// @accessor: A_PROTECT
CURSES_ACCESSOR(A_PROTECT)

// @accessor: A_INVIS
CURSES_ACCESSOR(A_INVIS)

// @accessor: A_ALTCHARSET
CURSES_ACCESSOR(A_ALTCHARSET)

// @accessor: A_CHARTEXT
CURSES_ACCESSOR(A_CHARTEXT)

/*
void JSSetLength(Local<String> property, Local<Value> value,
               const AccessorInfo& info) {
  ACCESSOR_GET_SELF(Line);
  HandleScope scope;
  uint32_t newsize = value->Uint32Value();
  self->value.resize(static_cast<std::string::size_type>(newsize));
}
*/


// @method: doupdate
// @description: Calls the underlying ncurses `doupdate()` routine.
CURSES_VOID_FUNC(Doupdate, doupdate)

// @method: refresh
// @description: Calls the underlying ncurses `refresh()` routine.
CURSES_VOID_FUNC(Refresh, refresh)

// @method: color_pair
// @param[pair]: #int the pair number
Handle<Value> CursesColorPair(const Arguments& args) {
  CHECK_ARGS(1);
  int32_t pairnum = args[0]->Int32Value();
  short pair = static_cast<short>(pairnum);  // NOLINT
  return scope.Close(Integer::New(COLOR_PAIR(pair)));
}

// @method: init_pair
// @param[pair]: #int the pair number
// @param[foreground]: #int the foreground color
// @param[background]: #int the background color
Handle<Value> CursesInitPair(const Arguments& args) {
  CHECK_ARGS(3);
  int32_t pairnum = args[0]->Int32Value();
  int32_t foreground = args[1]->Int32Value();
  int32_t background = args[2]->Int32Value();
  int ret = init_pair(static_cast<short>(pairnum),      // NOLINT
                      static_cast<short>(foreground),   // NOLINT
                      static_cast<short>(background));  // NOLINT
  if (ret == OK) {
    return scope.Close(Boolean::New(true));
  } else {
    return scope.Close(Boolean::New(false));
  }
}

// @method: move
// @param[y]: #int the row to move to
// @param[x]: #int the column to move to
// @description: This method moves the user-visible cursor.
Handle<Value> CursesMove(const Arguments& args) {
  if (args.Length() < 2) {
    return Undefined();
  }
  HandleScope scope;
  int y = static_cast<int>(args[0]->Int32Value());
  int x = static_cast<int>(args[1]->Int32Value());

  Local<Integer> ret = Integer::New(move(y, x));
  return scope.Close(ret);
}

// @method: newwin
// @param[nlines]: #int the number of lines in the window
// @param[ncols]: #int the number of columns in the window
// @param[begin_y]: #int the y-coordinate of the upper-left corner
// @param[begin_x]: #int the x-coordinate of the upper-left corner
// @description: This method creates a new curses window.
Handle<Value> CursesNewwin(const Arguments& args) {
  CHECK_ARGS(4);

  int nlines = static_cast<int>(args[0]->Int32Value());
  int ncols = static_cast<int>(args[1]->Int32Value());
  int begin_y = static_cast<int>(args[2]->Int32Value());
  int begin_x = static_cast<int>(args[3]->Int32Value());

  WINDOW *w = newwin(nlines, ncols, begin_y, begin_x);
  e::JSCursesWindow *cw = new e::JSCursesWindow(w);
  return scope.Close(cw->ToScript());
}
}

namespace e {
namespace js_curses {
bool Build(Handle<Object> obj) {
  HandleScope scope;

  // @accessor: OK
  // @description: The value of `OK` in the underlying obj implementation.
  NEW_INTEGER(obj, OK);

  // @accessor: ERR
  // @description: The value of `ERR` in the underlying obj implementation.
  NEW_INTEGER(obj, ERR);

  // @accessor: BUTTON1_CLICKED
  NEW_INTEGER(obj, BUTTON1_CLICKED);

  // @accessor: BUTTON1_DOUBLE_CLICKED
  NEW_INTEGER(obj, BUTTON1_DOUBLE_CLICKED);

  // @accessor: BUTTON1_PRESSED
  NEW_INTEGER(obj, BUTTON1_PRESSED);

  // @accessor: BUTTON1_RELEASED
  NEW_INTEGER(obj, BUTTON1_RELEASED);

  // @accessor: BUTTON1_RESERVED_EVENT
  NEW_INTEGER(obj, BUTTON1_RESERVED_EVENT);

  // @accessor: BUTTON1_TRIPLE_CLICKED
  NEW_INTEGER(obj, BUTTON1_TRIPLE_CLICKED);

  // @accessor: BUTTON2_CLICKED
  NEW_INTEGER(obj, BUTTON2_CLICKED);

  // @accessor: BUTTON2_DOUBLE_CLICKED
  NEW_INTEGER(obj, BUTTON2_DOUBLE_CLICKED);

  // @accessor: BUTTON2_PRESSED
  NEW_INTEGER(obj, BUTTON2_PRESSED);

  // @accessor: BUTTON2_RELEASED
  NEW_INTEGER(obj, BUTTON2_RELEASED);

  // @accessor: BUTTON2_RESERVED_EVENT
  NEW_INTEGER(obj, BUTTON2_RESERVED_EVENT);

  // @accessor: BUTTON2_TRIPLE_CLICKED
  NEW_INTEGER(obj, BUTTON2_TRIPLE_CLICKED);

  // @accessor: BUTTON3_CLICKED
  NEW_INTEGER(obj, BUTTON3_CLICKED);

  // @accessor: BUTTON3_DOUBLE_CLICKED
  NEW_INTEGER(obj, BUTTON3_DOUBLE_CLICKED);

  // @accessor: BUTTON3_PRESSED
  NEW_INTEGER(obj, BUTTON3_PRESSED);

  // @accessor: BUTTON3_RELEASED
  NEW_INTEGER(obj, BUTTON3_RELEASED);

  // @accessor: BUTTON3_RESERVED_EVENT
  NEW_INTEGER(obj, BUTTON3_RESERVED_EVENT);

  // @accessor: BUTTON3_TRIPLE_CLICKED
  NEW_INTEGER(obj, BUTTON3_TRIPLE_CLICKED);

  // @accessor: BUTTON4_CLICKED
  NEW_INTEGER(obj, BUTTON4_CLICKED);

  // @accessor: BUTTON4_DOUBLE_CLICKED
  NEW_INTEGER(obj, BUTTON4_DOUBLE_CLICKED);

  // @accessor: BUTTON4_PRESSED
  NEW_INTEGER(obj, BUTTON4_PRESSED);

  // @accessor: BUTTON4_RELEASED
  NEW_INTEGER(obj, BUTTON4_RELEASED);

  // @accessor: BUTTON4_RESERVED_EVENT
  NEW_INTEGER(obj, BUTTON4_RESERVED_EVENT);

  // @accessor: BUTTON4_TRIPLE_CLICKED
  NEW_INTEGER(obj, BUTTON4_TRIPLE_CLICKED);

  // @accessor: BUTTON_ALT
  NEW_INTEGER(obj, BUTTON_ALT);

  // @accessor: BUTTON_CTRL
  NEW_INTEGER(obj, BUTTON_CTRL);

  // @accessor: BUTTON_SHIFT
  NEW_INTEGER(obj, BUTTON_SHIFT);

  DECLARE_ACCESSOR(obj, COLOR_PAIRS);
  DECLARE_ACCESSOR(obj, COLORS);
  DECLARE_ACCESSOR(obj, COLS);
  DECLARE_ACCESSOR(obj, ESCDELAY);
  DECLARE_ACCESSOR(obj, LINES);
  DECLARE_ACCESSOR(obj, TABSIZE);
  DECLARE_ACCESSOR(obj, COLOR_BLACK);
  DECLARE_ACCESSOR(obj, COLOR_RED);
  DECLARE_ACCESSOR(obj, COLOR_GREEN);
  DECLARE_ACCESSOR(obj, COLOR_YELLOW);
  DECLARE_ACCESSOR(obj, COLOR_BLUE);
  DECLARE_ACCESSOR(obj, COLOR_MAGENTA);
  DECLARE_ACCESSOR(obj, COLOR_CYAN);
  DECLARE_ACCESSOR(obj, COLOR_WHITE);
  DECLARE_ACCESSOR(obj, A_NORMAL);
  DECLARE_ACCESSOR(obj, A_STANDOUT);
  DECLARE_ACCESSOR(obj, A_UNDERLINE);
  DECLARE_ACCESSOR(obj, A_REVERSE);
  DECLARE_ACCESSOR(obj, A_BLINK);
  DECLARE_ACCESSOR(obj, A_DIM);
  DECLARE_ACCESSOR(obj, A_BOLD);
  DECLARE_ACCESSOR(obj, A_PROTECT);
  DECLARE_ACCESSOR(obj, A_INVIS);
  DECLARE_ACCESSOR(obj, A_ALTCHARSET);
  DECLARE_ACCESSOR(obj, A_CHARTEXT);

  AddFunction(obj, "color_pair", &CursesColorPair);
  AddFunction(obj, "doupdate", &CursesDoupdate);
  AddFunction(obj, "init_pair", &CursesInitPair);
  AddFunction(obj, "move", &CursesMove);
  AddFunction(obj, "newwin", &CursesNewwin);
  AddFunction(obj, "refresh", &CursesRefresh);

  return true;
}
}
}
