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
  Handle<Value> JS_##name(const Arguments& args) {          \
    CHECK_ARGS(0);                                          \
    GET_SELF(JSCursesWindow);                               \
    return scope.Close(Integer::New(name(self->window_)));  \
  }

#define CURSES_STRING_FUNC(name)                                        \
  Handle<Value> JS_##name(const Arguments& args) {                      \
    CHECK_ARGS(1);                                                      \
    GET_SELF(JSCursesWindow);                                           \
    String::AsciiValue value(args[0]);                                  \
    return scope.Close(Integer::New(name(self->window_,                 \
                                         *value, value.length())));     \
  }

#define CURSES_INT_FUNC(name)                                           \
  Handle<Value> JS_##name(const Arguments& args) {                      \
    CHECK_ARGS(1);                                                      \
    GET_SELF(JSCursesWindow);                                           \
    int val = static_cast<int>(args[0]->Int32Value());                  \
    return scope.Close(Integer::New(name(self->window_, val)));         \
  }

#define CURSES_BOOL_FUNC(name)                                          \
  Handle<Value> JS_##name(const Arguments& args) {                      \
    CHECK_ARGS(1);                                                      \
    GET_SELF(JSCursesWindow);                                           \
    bool b = args[0]->BooleanValue();                                   \
    return scope.Close(Integer::New(name(self->window_, b)));           \
  }

#define CURSES_YX_FUNC(name)                                            \
  Handle<Value> JS_##name(const Arguments& args) {                      \
    CHECK_ARGS(2);                                                      \
    GET_SELF(JSCursesWindow);                                           \
    int y = static_cast<int>(args[0]->Int32Value());                    \
    int x = static_cast<int>(args[1]->Int32Value());                    \
    return scope.Close(Integer::New(name(self->window_, y, x)));        \
  }

#define CURSES_YX_STRING_FUNC(name)                                     \
  Handle<Value> JS_##name(const Arguments& args) {                      \
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
// @class: curses.Window
// @description: The JS representation of a `curses.WINDOW` object. In general,
//               the commands below correspond to the `w*` commands in curses,
//               so e.g. the `addstr` method actually maps to `waddstr(3)` in
//               the underlying curses implementation. Also note that curses
//               routines accepting strings use the `n` variant, so it's safe to
//               use strings containing null bytes.


// @method: addstr
// @param[str]: #string the string to draw
// @description: Adds a string at the current cursor location.
CURSES_STRING_FUNC(waddnstr);

// @method: attron
// @param[attrs]: #int The attributes to turn on.
// @description: Turns on the given attributes.
CURSES_INT_FUNC(wattron);

// @method: attroff
// @param[attrs]: #int The attributes to turn off.
// @description: Turns off the given attributes.
CURSES_INT_FUNC(wattroff);

// @method: attrset
// @param[attrs]: #int The attributes to set.
// @description: Sets the given attributes, and clears all others.
CURSES_INT_FUNC(wattrset);

// @method: clear
// @description: Clears the window.
CURSES_VOID_FUNC(wclear);

// @method: clrtobot
// @description: Clears from the cursor to the bottom of the window.
CURSES_VOID_FUNC(wclrtobot);

// @method: clrtoeol
// @description: Clears from the cursor to the end of the line.
CURSES_VOID_FUNC(wclrtoeol);

// @method: erase
// @description: Like `clear()`, but also calls `clearok()`.
CURSES_VOID_FUNC(werase);

// @method: getattrs
// @description: Gets the attributes of the current window.
CURSES_VOID_FUNC(getattrs);

// @method: getbegx
// @description: Returns the absolute x-coordinate of the origin of the current
//               window.

CURSES_VOID_FUNC(getbegx);

// @method: getbegy
// @description: Returns the absolute y-coordinate of the origin of the current
//               window.
CURSES_VOID_FUNC(getbegy);

// @method: getcurx
// @description: Returns the x-coordinate of the window's cursor.
CURSES_VOID_FUNC(getcurx);

// @method: getcury
// @description: Returns the y-coordinate of the window's cursor.
CURSES_VOID_FUNC(getcury);

// @method: getmaxx
// @description: Returns the maximum x-coordinate for the window.
CURSES_VOID_FUNC(getmaxx);

// @method: getmaxy
// @description: Returns the maximum y-coordinate for the window.
CURSES_VOID_FUNC(getmaxy);

// @method: getparx
// @description: Returns the x-coordinate of the window relative to its parent.
CURSES_VOID_FUNC(getparx);

// @method: getpary
// @description: Returns the y-coordinate of the window relative to its parent.
CURSES_VOID_FUNC(getpary);

// @method: move
// @param[y]: #int the y-coordinate to move to
// @param[x]: #int the x-coordinate to move to
// @description: Moves the cursor to the specified (y, x) coordinate.
CURSES_YX_FUNC(wmove);

// @method: mvaddstr
// @param[y]: #int the y-coordinate to move to
// @param[x]: #int the x-coordinate to move to
// @param[str]: #string the string to draw
// @description: Moves the cursor to the specified (y, x) coordinate, and then
//               draws a string (updating the cursor position).
CURSES_YX_STRING_FUNC(mvwaddnstr);

// @method: mvdelch
// @param[y]: #int the y-coordinate to move to
// @param[x]: #int the x-coordinate to move to
// @description: Moves the cursor to the specified (y, x) coordinate, and then
//               deletes a character. Following characters will be shifted left
//               as necessary.
CURSES_YX_FUNC(mvwdelch);

// @method: mvwin
// @param[y]: #int the y-coordinate to move the window to
// @param[x]: #int the x-coordinate to move the window to
// @description: Moves the origin of the current window.
CURSES_YX_FUNC(mvwin);

// @method: noutrefresh
// @description: Copy the contents of the window to the virtual screen.
CURSES_VOID_FUNC(wnoutrefresh);

// @method: redrawwin
// @description: Redraw the window.
CURSES_VOID_FUNC(redrawwin);

// @method: redrawln
// @param[beg_line]: #int the first corrupted line
// @param[num_lines]: #int the number of corrupted lines
// @description: Redraws multiple lines.
CURSES_YX_FUNC(wredrawln);

// @method: refresh
// @description: Rereshes the window (i.e. causes the windows contents to be
//               drawn to the screen).
CURSES_VOID_FUNC(wrefresh);

// @method: scrl
// @param[n]: #int the number of lines to scroll
// @description: Scrolls the window a given number of lines.
CURSES_INT_FUNC(wscrl);

// @method: scrollok
// @param[bf]: #bool is scrolling allowed?
// @description: Toggles scrollability for the window.
CURSES_BOOL_FUNC(scrollok);

// @method: setscrreg
// @param[top]: #int the top line in the scroll region
// @param[bot]: #int the bottom line in the scroll region
// @description: Establishes a section of the screen for scrolling.
CURSES_YX_FUNC(wsetscrreg);

// @method: standend
// @description: Ends standout mode.
CURSES_VOID_FUNC(wstandend);

// @method: standout
// @description: Begins standout mode.
CURSES_VOID_FUNC(wstandout);

// @method: insch
// @param[ch]: #char the character to insert
// @description: Inserts a character at the current cursor position, shifting
//               following characters to the right as necessary. If `ch` is more
//               than one character, it will be truncated to the first
//               character.
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

// @method: subwin
// @param[nlines]: #int the number of lines in the new window
// @param[ncols]: #int the number of columns in the new window
// @param[begin_y]: #int the y-coordinate of the origin of the new window
// @param[begin_x]: #int the x-coordinate of the origin of the new window
// @description: Creates a new subwindow of the current window.
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
