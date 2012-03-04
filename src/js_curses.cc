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
Handle<Value> CursesAddstr(const Arguments& args) {
  if (args.Length() < 1) {
    return Undefined();
  }
  HandleScope scope;
  Handle<Value> arg = args[0];
  String::Utf8Value value(arg);
  Local<Integer> ret = Integer::New(addnstr(*value, value.length()));
  return scope.Close(ret);
}

CURSES_VOID_FUNC(Clear, clear)
CURSES_VOID_FUNC(Clrtobot, clrtobot)
CURSES_VOID_FUNC(Clrtoeol, clrtoeol)
CURSES_VOID_FUNC(Delch, delch)
CURSES_VOID_FUNC(Doupdate, doupdate)
CURSES_VOID_FUNC(Erase, erase)

CURSES_WINDOW_FUNC(Getattrs, getattrs);
CURSES_WINDOW_FUNC(Getbegx, getbegx);
CURSES_WINDOW_FUNC(Getbegy, getbegy);
CURSES_WINDOW_FUNC(Getmaxx, getmaxx);
CURSES_WINDOW_FUNC(Getmaxy, getmaxy);
CURSES_WINDOW_FUNC(Getparx, getparx);
CURSES_WINDOW_FUNC(Getpary, getpary);
CURSES_WINDOW_FUNC(Redrawwin, redrawwin)

Handle<Value> CursesGetcurx(const Arguments& args) {
  HandleScope scope;
  int cx = getcurx(stdscr);
  Handle<Integer> curx = Integer::New(cx);
  return scope.Close(curx);
}

Handle<Value> CursesGetcury(const Arguments& args) {
  HandleScope scope;
  int cy = getcury(stdscr);
  Handle<Integer> cury = Integer::New(cy);
  return scope.Close(cury);
}

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

Handle<Value> CursesMvdelch(const Arguments& args) {
  if (args.Length() < 2) {
    return Undefined();
  }
  HandleScope scope;
  int y = static_cast<int>(args[0]->Int32Value());
  int x = static_cast<int>(args[1]->Int32Value());

  Local<Integer> ret = Integer::New(mvdelch(y, x));
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
  callbacks["addstr"] = &CursesAddstr;
  callbacks["clear"] = &CursesClear;
  callbacks["clrtobot"] = &CursesClrtobot;
  callbacks["clrtoeol"] = &CursesClrtoeol;
  callbacks["delch"] = &CursesDelch;
  callbacks["doupdate"] = &CursesDoupdate;
  callbacks["erase"] = &CursesErase;
  callbacks["getattrs"] = &CursesGetattrs;
  callbacks["getbegx"] = &CursesGetbegx;
  callbacks["getbegy"] = &CursesGetbegy;
  callbacks["getcurx"] = &CursesGetcurx;
  callbacks["getcury"] = &CursesGetcury;
  callbacks["getmaxx"] = &CursesGetmaxx;
  callbacks["getmaxy"] = &CursesGetmaxy;
  callbacks["getparx"] = &CursesGetparx;
  callbacks["getpary"] = &CursesGetpary;
  callbacks["move"] = &CursesMove;
  callbacks["mvdelch"] = &CursesMvdelch;
  callbacks["newwin"] = &CursesNewwin;
  callbacks["redrawwin"] = &CursesRedrawwin;
  return callbacks;
}
}
