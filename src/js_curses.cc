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

CURSES_ACCESSOR(COLOR_PAIRS);
CURSES_ACCESSOR(COLORS);
CURSES_ACCESSOR(COLS);
CURSES_ACCESSOR(ESCDELAY);
CURSES_ACCESSOR(LINES);
CURSES_ACCESSOR(TABSIZE);

// colors
CURSES_ACCESSOR(COLOR_BLACK);
CURSES_ACCESSOR(COLOR_RED);
CURSES_ACCESSOR(COLOR_GREEN);
CURSES_ACCESSOR(COLOR_YELLOW);
CURSES_ACCESSOR(COLOR_BLUE);
CURSES_ACCESSOR(COLOR_MAGENTA);
CURSES_ACCESSOR(COLOR_CYAN);
CURSES_ACCESSOR(COLOR_WHITE);

// attributes
CURSES_ACCESSOR(A_NORMAL);
CURSES_ACCESSOR(A_STANDOUT);
CURSES_ACCESSOR(A_UNDERLINE);
CURSES_ACCESSOR(A_REVERSE);
CURSES_ACCESSOR(A_BLINK);
CURSES_ACCESSOR(A_DIM);
CURSES_ACCESSOR(A_BOLD);
CURSES_ACCESSOR(A_PROTECT);
CURSES_ACCESSOR(A_INVIS);
CURSES_ACCESSOR(A_ALTCHARSET);
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


CURSES_VOID_FUNC(Doupdate, doupdate)
CURSES_VOID_FUNC(Refresh, refresh)

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
