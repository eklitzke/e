// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>


#include <glog/logging.h>

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/bind.hpp>

#include <curses.h>
#include <term.h>
#include <termios.h>

#include "./keycode.h"
#include "./termios_window.h"

namespace e {
TermiosWindow::TermiosWindow(const std::string &script_name)
    :state_(script_name), term_in_(io_service_) {
  window_ = initscr();
  refresh();

  noecho();
  nonl();  // don't turn LF into CRLF
  cbreak();  // read characters one at a time
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

TermiosWindow::~TermiosWindow() {
  endwin();
}

/* Add a read-event to the ioservice loop that will fire as soon as keyboard
 * input is available.
 */
void TermiosWindow::EstablishReadLoop() {
  term_in_.async_read_some(
      boost::asio::null_buffers(),
      boost::bind(&TermiosWindow::OnRead, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

bool TermiosWindow::HandleKey(KeyCode *keycode) {
  bool result = state_.HandleKey(keycode);
  if (result)
    refresh();
  return result;
}

void TermiosWindow::OnRead(const boost::system::error_code& error,
                           std::size_t bytes_transferred) {
  // there should be at least one byte to read (and possibly more); keep reading
  // bytes until getch() returns ERR
  bool keep_going = true;
  while (true) {
    int key = getch();
    if (key == ERR)
      break;

    KeyCode keycode = e::keycode::curses_code_to_keycode(key);
    keep_going = HandleKey(&keycode);
    if (!keep_going)
      break;
  }
  if (keep_going)
    EstablishReadLoop();
}

void TermiosWindow::Loop() {
  state_.RunScript(boost::bind(&TermiosWindow::InnerLoop, this));
}

void TermiosWindow::InnerLoop() {
  EstablishReadLoop();
  io_service_.run();
}
}
