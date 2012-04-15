// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./curses_window.h"

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <term.h>
#include <termios.h>
#include <v8.h>
#include <wchar.h>

#ifdef USE_NCURSESW
#ifdef PLATFORM_LINUX
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif  // _X_OPEN_SOURCE_EXTENDED
#include <ncursesw/curses.h>
#else
#include <curses.h>
#endif  // PLATFORM_LINUX
#else  // USE_NCURSESW
#include <curses.h>  // NOLINT
#endif  // USE_NCURSESW

#include <functional>

#include "./assert.h"
#include "./curses_low_level.h"
#include "./flags.h"
#include "./io_service.h"
#include "./js_curses_window.h"
#include "./keycode.h"
#include "./logging.h"

using v8::Context;
using v8::Local;
using v8::String;
using v8::Object;

namespace e {
namespace {
bool UseAsio() {
  return !vm.count("without-boost-asio");
}
}

CursesWindow::CursesWindow(const std::vector<std::string> &scripts,
                           const std::vector<std::string> &files)
    :state_(scripts, files), args_(files),
     term_in_(io_service) {
}

void CursesWindow::Initialize() {
  WINDOW *window = initscr();

  InitializeCurses();

  keypad(window, TRUE);
  clearok(window, TRUE);
  notimeout(window, TRUE);
  if (UseAsio()) {
    nodelay(window, true);
  }

  struct termios ttystate;
  tcgetattr(STDIN_FILENO, &ttystate);
  ttystate.c_iflag &= ~IXON;  // allow capturs Ctrl-Q/Ctrl-S
  tcsetattr(0, TCSANOW, &ttystate);

  if (UseAsio()) {
    term_in_.assign(STDIN_FILENO);
  }
}

/* Add a read-event to the ioservice loop that will fire as soon as keyboard
 * input is available.
 */
void CursesWindow::EstablishReadLoop() {
  term_in_.async_read_some(
      boost::asio::null_buffers(),
      std::bind(&CursesWindow::OnRead, this,
                  std::placeholders::_1,
                  std::placeholders::_2));
}

bool CursesWindow::HandleKey(KeyCode *keycode) {
  return state_.HandleKey(keycode);
}

void CursesWindow::OnRead(const boost::system::error_code& error,
                           std::size_t bytes_transferred) {
  if (error) {
    LOG(FATAL, "boot::asio error ", error.message().c_str());
  } else {
    InnerOnRead();
  }
}

bool CursesWindow::InnerOnRead() {
  // there should be at least one byte to read (and possibly more); keep reading
  // bytes until getch() returns ERR
  bool keep_going = true;
  while (true) {
#ifdef USE_NCURSESW
    wint_t wch;
    int ret = get_wch(&wch);
    if (ret == ERR) {
      break;
    }
    bool is_keycode = (ret == KEY_CODE_YES);
#else
    int ch = static_cast<wint_t>(getch());
    if (ch == ERR) {
      break;
    }
    wint_t wch = static_cast<wint_t>(ch);
    bool is_keycode = (wch >= 256);
#endif
    KeyCode *keycode = e::keycode::CursesToKeycode(wch, is_keycode);
    if (keycode->IsPrintable()) {
      LOG(DBG, "read '%c' from keyboard (code %d)",
          static_cast<char>(wch), wch);
    } else {
      LOG(DBG, "read code %d from keyboard", wch);
    }

    keep_going = HandleKey(keycode);
    if (!keep_going) {
      break;
    }
  }
  if (keep_going) {
    v8::V8::IdleNotification();  // tell v8 we're idle (it may want to GC)
    EstablishReadLoop();
  }
  return keep_going;
}

void CursesWindow::Loop() {
  state_.Run(std::bind(&CursesWindow::InnerLoop, this));
}

void CursesWindow::InnerLoop() {
  Initialize();

  // Once initscr() has been called, we can create an object to hold the stdscr
  // pointer.
  JSCursesWindow jcw(stdscr);
  Local<Context> c = Context::GetCurrent();
  Local<Object> curses = c->Global()->Get(
      String::NewSymbol("curses"))->ToObject();
  curses->Set(String::NewSymbol("stdscr"), jcw.ToScript());

  state_.GetListener()->Dispatch("load");

  LOG(INFO, "waiting for keypresses...");
  if (UseAsio()) {
    EstablishReadLoop();
    io_service.run();
  } else {
    while (InnerOnRead()) { }
  }
}
}
