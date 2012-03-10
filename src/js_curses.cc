// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <curses.h>
#include <v8.h>
#include <glog/logging.h>
#include <glog/log_severity.h>

#include <map>
#include <string>

#include "./js.h"
#include "./js_curses_window.h"

using v8::AccessorInfo;
using v8::Arguments;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Integer;
using v8::Object;
using v8::String;
using v8::Undefined;
using v8::Value;

#define CURSES_VOID_FUNC(capname, func)\
Handle<Value> Curses ## capname(const Arguments& args) {  \
  HandleScope scope;\
  Local<Integer> ret = Integer::New(func());\
  return scope.Close(ret);\
}

#define CURSES_WINDOW_FUNC(capname, func)\
Handle<Value> Curses ## capname(const Arguments& args) {  \
  HandleScope scope;\
  Local<Integer> ret = Integer::New(func(stdscr));\
  return scope.Close(ret);\
}

#define CURSES_ACCESSOR(macroname)                                      \
  Handle<Value> JS_##macroname (Local<String> property,                 \
                                const AccessorInfo& info) {             \
    HandleScope scope;                                                  \
    return scope.Close(Integer::New(macroname));                        \
  }

#define DECLARE_ACCESSOR(macroname)             \
  accessors[#macroname] = JS_##macroname;

namespace e {
// @class: curses
// @description: a low-level interface to ncurses

// @accessor: OK
// @returns: #int the OK value for curses
//
// This is set in state.cc

// @accessor: ERR
// @returns: #int the error value for curses
//
// This is set in state.cc

// @accessor: COLOR_PAIRS
// @returns: #int the number of color pairs the terminal supports
CURSES_ACCESSOR(COLOR_PAIRS);

// @accessor: COLOR_PAIRS
// @returns: #int the number of colors the terminal supports
CURSES_ACCESSOR(COLORS);

// @accessor: COLS
// @returns: #int the width of the screen (i.e. the number of columns)
CURSES_ACCESSOR(COLS);

// @accessor: ESCDELAY
// @returns: #int the number of milliseconds curses will wait after reading
//           an escape character
CURSES_ACCESSOR(ESCDELAY);

// @accessor: ESCDELAY
// @returns: #int the height of the screen (i.e. the number of lines)
CURSES_ACCESSOR(LINES);

// @accessor: TABSIZE
// @returns: #int the number of columns in a tab character
CURSES_ACCESSOR(TABSIZE);

// @accessor: COLOR_BLACK
CURSES_ACCESSOR(COLOR_BLACK);

// @accessor: COLOR_RED
CURSES_ACCESSOR(COLOR_RED);

// @accessor: COLOR_GREEN
CURSES_ACCESSOR(COLOR_GREEN);

// @accessor: COLOR_YELLOW
CURSES_ACCESSOR(COLOR_YELLOW);

// @accessor: COLOR_BLUE
CURSES_ACCESSOR(COLOR_BLUE);

// @accessor: COLOR_MAGENTA
CURSES_ACCESSOR(COLOR_MAGENTA);

// @accessor: COLOR_CYAN
CURSES_ACCESSOR(COLOR_CYAN);

// @accessor: COLOR_WHITE
CURSES_ACCESSOR(COLOR_WHITE);

// @accessor: A_NORMAL
CURSES_ACCESSOR(A_NORMAL);

// @accessor: A_STANDOUT
CURSES_ACCESSOR(A_STANDOUT);

// @accessor: A_UNDERLINE
CURSES_ACCESSOR(A_UNDERLINE);

// @accessor: A_REVERSE
CURSES_ACCESSOR(A_REVERSE);

// @accessor: A_BLINK
CURSES_ACCESSOR(A_BLINK);

// @accessor: A_DIM
CURSES_ACCESSOR(A_DIM);

// @accessor: A_BOLD
CURSES_ACCESSOR(A_BOLD);

// @accessor: A_PROTECT
CURSES_ACCESSOR(A_PROTECT);

// @accessor: A_INVIS
CURSES_ACCESSOR(A_INVIS);

// @accessor: A_ALTCHARSET
CURSES_ACCESSOR(A_ALTCHARSET);

// @accessor: A_CHARTEXT
CURSES_ACCESSOR(A_CHARTEXT);

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
// @description: calls the curses `doupdate()` routine
CURSES_VOID_FUNC(Doupdate, doupdate)

// @method: refresh
// @description: calls the curses `refresh()` routine
CURSES_VOID_FUNC(Refresh, refresh)

// @method: move
// @param[y]: #int the row to move to
// @param[x]: #int the column to move to
// @description: this method moves the user-visible cursor
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
// @description: this method creates a new curses window
Handle<Value> CursesNewwin(const Arguments& args) {
  CHECK_ARGS(4);

  int nlines = static_cast<int>(args[0]->Int32Value());
  int ncols = static_cast<int>(args[1]->Int32Value());
  int begin_y = static_cast<int>(args[2]->Int32Value());
  int begin_x = static_cast<int>(args[3]->Int32Value());

  LOG(INFO) << "calling newwin";
  WINDOW *w = newwin(nlines, ncols, begin_y, begin_x);
  LOG(INFO) << "new window is " << w;
  JSCursesWindow *cw = new JSCursesWindow(w);
  return scope.Close(cw->ToScript());
}

std::map<std::string, js::JSCallback> GetCursesCallbacks() {
  std::map<std::string, js::JSCallback> callbacks;
  callbacks["doupdate"] = &CursesDoupdate;
  callbacks["move"] = &CursesMove;
  callbacks["newwin"] = &CursesNewwin;
  callbacks["refresh"] = &CursesRefresh;
  return callbacks;
}

std::map<std::string, js::JSAccessor> GetCursesAccessors() {
  std::map<std::string, js::JSAccessor> accessors;
  DECLARE_ACCESSOR(COLOR_PAIRS);
  DECLARE_ACCESSOR(COLORS);
  DECLARE_ACCESSOR(COLS);
  DECLARE_ACCESSOR(ESCDELAY);
  DECLARE_ACCESSOR(LINES);
  DECLARE_ACCESSOR(TABSIZE);
  DECLARE_ACCESSOR(COLOR_BLACK);
  DECLARE_ACCESSOR(COLOR_RED);
  DECLARE_ACCESSOR(COLOR_GREEN);
  DECLARE_ACCESSOR(COLOR_YELLOW);
  DECLARE_ACCESSOR(COLOR_BLUE);
  DECLARE_ACCESSOR(COLOR_MAGENTA);
  DECLARE_ACCESSOR(COLOR_CYAN);
  DECLARE_ACCESSOR(COLOR_WHITE);
  DECLARE_ACCESSOR(A_NORMAL);
  DECLARE_ACCESSOR(A_STANDOUT);
  DECLARE_ACCESSOR(A_UNDERLINE);
  DECLARE_ACCESSOR(A_REVERSE);
  DECLARE_ACCESSOR(A_BLINK);
  DECLARE_ACCESSOR(A_DIM);
  DECLARE_ACCESSOR(A_BOLD);
  DECLARE_ACCESSOR(A_PROTECT);
  DECLARE_ACCESSOR(A_INVIS);
  DECLARE_ACCESSOR(A_ALTCHARSET);
  DECLARE_ACCESSOR(A_CHARTEXT);

  return accessors;
}

}
