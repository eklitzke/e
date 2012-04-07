// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./curses_window.h"

#include <glog/logging.h>

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/bind.hpp>
#include <v8.h>

#include <term.h>
#include <termios.h>

#include "./assert.h"
#include "./curses_low_level.h"
#include "./flags.h"
#include "./io_service.h"
#include "./keycode.h"
#include "./js_curses_window.h"

namespace e {
namespace {
bool UseAsio() {
  return !vm().count("without-boost-asio");
}
}

CursesWindow::CursesWindow(const std::vector<std::string> &scripts,
                           const std::vector<std::string> &files)
    :state_(scripts, files), window_(nullptr), args_(files),
     term_in_(*GetIOService()) {
}

void CursesWindow::Initialize() {
  window_ = initscr();

  InitializeCurses();

  keypad(window_, TRUE);
  clearok(window_, TRUE);
  notimeout(window_, TRUE);
  if (UseAsio()) {
    nodelay(window_, true);
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
      boost::bind(&CursesWindow::OnRead, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

bool CursesWindow::HandleKey(KeyCode *keycode) {
  return state_.HandleKey(keycode);
}

void CursesWindow::OnRead(const boost::system::error_code& error,
                           std::size_t bytes_transferred) {
  if (error) {
    LOG(FATAL) << "boot::asio error " << error.message();
  } else {
    InnerOnRead();
  }
}

bool CursesWindow::InnerOnRead() {
  // there should be at least one byte to read (and possibly more); keep reading
  // bytes until getch() returns ERR
  bool keep_going = true;
  while (true) {
    wint_t wch;
    int ret = get_wch(&wch);
    if (ret == ERR)
      break;

    KeyCode *keycode = e::keycode::curses_to_keycode(wch, ret == KEY_CODE_YES);
    keep_going = HandleKey(keycode);
    if (!keep_going)
      break;
  }
  if (keep_going) {
    v8::V8::IdleNotification();  // tell v8 we're idle (it may want to GC)
    EstablishReadLoop();
  }
  return keep_going;
}

void CursesWindow::Loop() {
  state_.Run(boost::bind(&CursesWindow::InnerLoop, this, _1));
}

void CursesWindow::InnerLoop(v8::Persistent<v8::Context> c) {
  Initialize();

  // Once initscr() has been called, we can create an object to hold the stdscr
  // pointer.
  JSCursesWindow jcw(stdscr);
  Local<Object> curses = c->Global()->Get(
      String::NewSymbol("curses"))->ToObject();
  curses->Set(String::NewSymbol("stdscr"), jcw.ToScript());

  state_.GetListener()->Dispatch("load");

  if (UseAsio()) {
    EstablishReadLoop();
    GetIOService()->run();
  } else {
    while (InnerOnRead()) { }
  }
}
}
