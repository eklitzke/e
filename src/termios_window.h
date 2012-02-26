// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_TERMIOS_WINDOW_H_
#define SRC_TERMIOS_WINDOW_H_

#include <string>

#include <v8.h>

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#include <curses.h>

#include "./keycode.h"
#include "./state.h"

namespace e {
class TermiosWindow {
 public:
  explicit TermiosWindow(const std::string &script_name);
  ~TermiosWindow();

  void Loop();
 private:
  State state_;
  WINDOW *window_;
  boost::asio::io_service io_service_;
  boost::asio::posix::stream_descriptor term_in_;

  void Initialize();
  void InnerLoop();
  void OnRead(const boost::system::error_code& error, std::size_t bytes_transferred);
  bool HandleKey(const KeyCode &);
  void EstablishReadLoop();
};
}

#endif  // SRC_TERMIOS_WINDOW_H_
