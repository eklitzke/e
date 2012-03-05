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

namespace e {

Handle<Value> JSGetColorPairs(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  return scope.Close(Integer::New(COLOR_PAIRS));
}

Handle<Value> JSGetColors(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  return scope.Close(Integer::New(COLORS));
}

Handle<Value> JSGetCols(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  return scope.Close(Integer::New(COLS));
}

Handle<Value> JSGetEscDelay(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  return scope.Close(Integer::New(ESCDELAY));
}

Handle<Value> JSGetLines(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  return scope.Close(Integer::New(LINES));
}

Handle<Value> JSGetTabSize(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  return scope.Close(Integer::New(TABSIZE));
}

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
}
