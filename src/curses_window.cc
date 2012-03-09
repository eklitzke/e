// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./curses_window.h"

#include <glog/logging.h>

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/bind.hpp>
#include <v8.h>

#include <curses.h>
#include <term.h>
#include <termios.h>

#include "./keycode.h"
#include "./js_curses_window.h"

namespace e {
CursesWindow::CursesWindow(bool load_core,
                           const std::vector<std::string> &scripts,
                           const std::vector<std::string> &files)
    :state_(load_core, scripts, files), window_(nullptr), args_(files), term_in_(io_service_) {
}

CursesWindow::~CursesWindow() {
  if (window_ != nullptr)
    endwin();
}

void CursesWindow::Initialize() {
  window_ = initscr();

  start_color();
  use_default_colors();  // ncurses extension!
  noecho();
  nonl();  // don't turn LF into CRLF
  raw();  // read characters one at a time, and allow Ctrl-C, Ctl-Z, etc.
  keypad(window_, TRUE);
  clearok(window_, TRUE);
  notimeout(window_, TRUE);
  nodelay(window_, true);

  struct termios ttystate;
  tcgetattr(STDIN_FILENO, &ttystate);
  ttystate.c_iflag &= ~IXON;  // allow capturs Ctrl-Q/Ctrl-S
  tcsetattr(0, TCSANOW, &ttystate);

  term_in_.assign(STDIN_FILENO);
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
  // there should be at least one byte to read (and possibly more); keep reading
  // bytes until getch() returns ERR
  bool keep_going = true;
  while (true) {
    int key = getch();
    if (key == ERR)
      break;

    KeyCode *keycode = e::keycode::curses_code_to_keycode(key);
    keep_going = HandleKey(keycode);
    if (!keep_going)
      break;
  }
  if (keep_going) {
	v8::V8::IdleNotification(); // tell v8 we're idle (it may want to GC)
    EstablishReadLoop();
  }
}

void CursesWindow::Loop() {
  state_.LoadScript(true, boost::bind(&CursesWindow::InnerLoop, this, _1));
}

void CursesWindow::InnerLoop(v8::Persistent<v8::Context> c) {
  std::vector<Handle<Value> > args;
  Initialize();

  // Once initscr() has been called, we can create an object to hold the stdscr
  // pointer.
  JSCursesWindow jcw(stdscr);
  Local<Object> curses = c->Global()->Get(String::NewSymbol("curses"))->ToObject();
  curses->Set(String::NewSymbol("stdscr"), jcw.ToScript());

  state_.GetListener()->Dispatch("load", c->Global(), args);

  EstablishReadLoop();
  io_service_.run();
}
}
