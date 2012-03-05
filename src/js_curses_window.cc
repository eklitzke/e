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

#define CURSES_VOID_FUNC(name)                              \
  Handle<Value> JS_##name (const Arguments& args) {         \
    CHECK_ARGS(0);                                          \
    GET_SELF(JSCursesWindow);                               \
    return scope.Close(Integer::New(name(self->window_)));  \
  }

#define CURSES_STRING_FUNC(name)                                        \
  Handle<Value> JS_##name (const Arguments& args) {                     \
    CHECK_ARGS(1);                                                      \
    GET_SELF(JSCursesWindow);                                           \
    String::AsciiValue value(args[0]);                                  \
    return scope.Close(Integer::New(name(self->window_,                 \
                                         *value, value.length())));     \
  }

#define CURSES_INT_FUNC(name)                                           \
  Handle<Value> JS_##name (const Arguments& args) {                     \
    CHECK_ARGS(1);                                                      \
    GET_SELF(JSCursesWindow);                                           \
    int val = static_cast<int>(args[0]->Int32Value());                  \
    return scope.Close(Integer::New(name(self->window_, val)));         \
  }

#define CURSES_BOOL_FUNC(name)                                          \
  Handle<Value> JS_##name (const Arguments& args) {                     \
    CHECK_ARGS(1);                                                      \
    GET_SELF(JSCursesWindow);                                           \
    bool b = args[0]->BooleanValue();                                   \
    return scope.Close(Integer::New(name(self->window_, b)));           \
  }

#define CURSES_YX_FUNC(name)                                            \
  Handle<Value> JS_##name (const Arguments& args) {                     \
    CHECK_ARGS(2);                                                      \
    GET_SELF(JSCursesWindow);                                           \
    int y = static_cast<int>(args[0]->Int32Value());                    \
    int x = static_cast<int>(args[1]->Int32Value());                    \
    return scope.Close(Integer::New(name(self->window_, y, x)));        \
  }

#define CURSES_YX_STRING_FUNC(name)                                     \
  Handle<Value> JS_##name (const Arguments& args) {                     \
    CHECK_ARGS(3);                                                      \
    GET_SELF(JSCursesWindow);                                           \
    int y = static_cast<int>(args[0]->Int32Value());                    \
    int x = static_cast<int>(args[1]->Int32Value());                    \
    String::AsciiValue value(args[2]);                                  \
    return scope.Close(Integer::New(name(                               \
        self->window_, y, x, *value, value.length())));                 \
  }

namespace e {
JSCursesWindow::JSCursesWindow(WINDOW *win)
    :window_(win) {
  LOG(INFO) << "creating new window, *win = " << win;
  idlok(win, true);
  wnoutrefresh(win);
}

JSCursesWindow::~JSCursesWindow() {
  LOG(INFO) << "deleting window " << window_;
  if (window_ != nullptr)
    delwin(window_);
}

namespace {
///////////////////////////////////////////////////////
// MACRO                               EXPORTED NAME //
///////////////////////////////////////////////////////
CURSES_STRING_FUNC(waddnstr);       // addstr
CURSES_INT_FUNC(wattron);           // attron
CURSES_INT_FUNC(wattroff);          // attroff
CURSES_INT_FUNC(wattrset);          // attrset
CURSES_VOID_FUNC(wclear);           // clear
CURSES_VOID_FUNC(wclrtobot);        // clrtobot
CURSES_VOID_FUNC(wclrtoeol);        // clrtoeol
CURSES_VOID_FUNC(werase);           // erase
CURSES_VOID_FUNC(getattrs);         // getattrs
CURSES_VOID_FUNC(getbegx);          // getbegx
CURSES_VOID_FUNC(getbegy);          // getbegy
CURSES_VOID_FUNC(getcurx);          // getcurx
CURSES_VOID_FUNC(getcury);          // getcury
CURSES_VOID_FUNC(getmaxx);          // getmaxx
CURSES_VOID_FUNC(getmaxy);          // getmaxy
CURSES_VOID_FUNC(getparx);          // getparx
CURSES_VOID_FUNC(getpary);          // getpary
CURSES_YX_FUNC(wmove);              // move
CURSES_YX_STRING_FUNC(mvwaddnstr);  // mvaddstr
CURSES_YX_FUNC(mvwdelch);           // mvdelch
CURSES_YX_FUNC(mvwin);              // mvwin
CURSES_VOID_FUNC(wnoutrefresh);     // noutrefresh
CURSES_VOID_FUNC(redrawwin);        // redrawwin
CURSES_YX_FUNC(wredrawln);          // redrawln
CURSES_VOID_FUNC(wrefresh);         // refresh
CURSES_INT_FUNC(wscrl);             // scrl
CURSES_BOOL_FUNC(scrollok);         // scrollok
CURSES_YX_FUNC(wsetscrreg);         // setscrreg
CURSES_VOID_FUNC(wstandend);        // standend
CURSES_VOID_FUNC(wstandout);        // standout

Handle<Value> JS_winsch(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(JSCursesWindow);                                           \
  String::AsciiValue value(args[0]);                                  \
  assert(value.length() == 1);

  // a chtype is wider than a char, so cast to a char before casting to chtype
  char char_val = static_cast<char>(**value);
  chtype chtype_val = static_cast<chtype>(char_val);
  return scope.Close(Integer::New(winsch(self->window_, chtype_val)));
}

Handle<Value> JSSubwin(const Arguments& args) {
  CHECK_ARGS(4);

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

Persistent<ObjectTemplate> templ;

// Create a raw template
Handle<ObjectTemplate> MakeTemplate() {
  HandleScope scope;
  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->SetInternalFieldCount(1);
  js::AddTemplateFunction(result, "addstr", JS_waddnstr);
  js::AddTemplateFunction(result, "attron", JS_wattron);
  js::AddTemplateFunction(result, "attroff", JS_wattroff);
  js::AddTemplateFunction(result, "attrset", JS_wattroff);
  js::AddTemplateFunction(result, "erase", JS_werase);
  js::AddTemplateFunction(result, "clear", JS_wclear);
  js::AddTemplateFunction(result, "clrtobot", JS_wclrtobot);
  js::AddTemplateFunction(result, "clrtoeol", JS_wclrtoeol);
  js::AddTemplateFunction(result, "getattrs", JS_getattrs);
  js::AddTemplateFunction(result, "getbegx", JS_getbegx);
  js::AddTemplateFunction(result, "getbegy", JS_getbegy);
  js::AddTemplateFunction(result, "getcurx", JS_getcurx);
  js::AddTemplateFunction(result, "getcury", JS_getcury);
  js::AddTemplateFunction(result, "getmaxx", JS_getmaxx);
  js::AddTemplateFunction(result, "getmaxy", JS_getmaxy);
  js::AddTemplateFunction(result, "getparx", JS_getparx);
  js::AddTemplateFunction(result, "getpary", JS_getpary);
  js::AddTemplateFunction(result, "insch", JS_winsch);
  js::AddTemplateFunction(result, "move", JS_wmove);
  js::AddTemplateFunction(result, "mvaddstr", JS_mvwaddnstr);
  js::AddTemplateFunction(result, "mvdelch", JS_mvwdelch);
  js::AddTemplateFunction(result, "mvwin", JS_mvwin);
  js::AddTemplateFunction(result, "noutrefresh", JS_wnoutrefresh);
  js::AddTemplateFunction(result, "refresh", JS_wrefresh);
  js::AddTemplateFunction(result, "redrawwin", JS_redrawwin);
  js::AddTemplateFunction(result, "redrawln", JS_wredrawln);
  js::AddTemplateFunction(result, "scrollok", JS_scrollok);
  js::AddTemplateFunction(result, "scrl", JS_wscrl);
  js::AddTemplateFunction(result, "setscrreg", JS_wsetscrreg);
  js::AddTemplateFunction(result, "standend", JS_wstandend);
  js::AddTemplateFunction(result, "standout", JS_wstandout);
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
