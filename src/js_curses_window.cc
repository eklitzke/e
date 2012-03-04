// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//

#include "./js_curses_window.h"

#include <glog/logging.h>
#include <v8.h>

#include <cassert>
#include <string>
#include <vector>

#include "./js.h"

using v8::AccessorInfo;
using v8::Arguments;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Integer;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Undefined;
using v8::Value;

namespace e {
JSCursesWindow::JSCursesWindow(WINDOW *win)
    :window_(win) {
  LOG(INFO) << "creating new window, *win = " << win;
  wnoutrefresh(win);
}

JSCursesWindow::~JSCursesWindow() {
  LOG(INFO) << "deleting window " << window_;
  if (window_ != nullptr)
    delwin(window_);
}

namespace {
Handle<Value> JSAddstr(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(JSCursesWindow);

  Handle<Value> arg = args[0];
  String::AsciiValue value(arg);
  return scope.Close(Integer::New(waddnstr(self->window_, *value, value.length())));
}

Handle<Value> JSMove(const Arguments& args) {
  CHECK_ARGS(2);
  GET_SELF(JSCursesWindow);

  int y = static_cast<int>(args[0]->Int32Value());
  int x = static_cast<int>(args[1]->Int32Value());
  return scope.Close(Integer::New(wmove(self->window_, y, x)));
}

Handle<Value> JSMvaddstr(const Arguments& args) {
  CHECK_ARGS(3);
  GET_SELF(JSCursesWindow);

  int y = static_cast<int>(args[0]->Int32Value());
  int x = static_cast<int>(args[1]->Int32Value());
  Handle<Value> val = args[2];
  String::AsciiValue value(val);
  return scope.Close(Integer::New(mvwaddnstr(self->window_, y, x, *value, value.length())));
}

Handle<Value> JSMvwin(const Arguments& args) {
  CHECK_ARGS(2);
  GET_SELF(JSCursesWindow);

  int y = static_cast<int>(args[0]->Int32Value());
  int x = static_cast<int>(args[1]->Int32Value());

  return scope.Close(Integer::New(mvwin(self->window_, y, x)));
}

Handle<Value> JSSubwin(const Arguments& args) {
  CHECK_ARGS(4);
  //GET_SELF(JSCursesWindow);

  JSCursesWindow *other = Unwrap<JSCursesWindow>(args.Holder());
  int nlines = static_cast<int>(args[0]->Int32Value());
  int ncols = static_cast<int>(args[1]->Int32Value());
  int begin_y = static_cast<int>(args[2]->Int32Value());
  int begin_x = static_cast<int>(args[3]->Int32Value());

  LOG(INFO) << "calling subwin";
  WINDOW *w = subwin(other->window_, nlines, ncols, begin_y, begin_x);
  LOG(INFO) << "new window is " << w;
  JSCursesWindow *cw = new JSCursesWindow(w);
  return scope.Close(cw->ToScript());
}

Handle<Value> JSRefresh(const Arguments& args) {
  CHECK_ARGS(0);
  GET_SELF(JSCursesWindow);
  wrefresh(self->window_);
  return scope.Close(Undefined());
}

Persistent<ObjectTemplate> templ;

// Create a raw template
Handle<ObjectTemplate> MakeTemplate() {
  HandleScope scope;
  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->SetInternalFieldCount(1);
  js::AddTemplateFunction(result, "addstr", JSAddstr);
  js::AddTemplateFunction(result, "move", JSMove);
  js::AddTemplateFunction(result, "mvaddstr", JSMvaddstr);
  js::AddTemplateFunction(result, "mvwin", JSMvwin);
  js::AddTemplateFunction(result, "refresh", JSRefresh);
  js::AddTemplateFunction(result, "subwin", JSSubwin);
  return scope.Close(result);
}
}

Handle<Value> JSCursesWindow::ToScript() {
  HandleScope scope;
  if (templ.IsEmpty()) {
    Handle<ObjectTemplate> raw_template = MakeTemplate();
    templ = Persistent<ObjectTemplate>::New(raw_template);
  }
  Handle<Object> cw = templ->NewInstance();
  assert(cw->InternalFieldCount() == 1);
  cw->SetInternalField(0, External::New(this));
  return scope.Close(cw);
}
}
