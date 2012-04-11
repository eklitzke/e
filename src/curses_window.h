// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_CURSES_WINDOW_H_
#define SRC_CURSES_WINDOW_H_

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#ifdef PLATFORM_LINUX
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#include <ncursesw/curses.h>
#else
#include <curses.h>
#endif  // PLATFORM_LINUX

#include <string>
#include <vector>

#include "./keycode.h"
#include "./state.h"

namespace e {
class CursesWindow {
 public:
  explicit CursesWindow(const std::vector<std::string> &scripts,
                        const std::vector<std::string> &files);
  void Initialize();
  void Loop();
 private:
  State state_;
  WINDOW *window_;
  std::vector<std::string> args_;
  boost::asio::posix::stream_descriptor term_in_;

  void InnerLoop();

  void OnRead(const boost::system::error_code&, std::size_t);
  bool InnerOnRead();
  bool HandleKey(KeyCode *k);
  void EstablishReadLoop();
};
}

#endif  // SRC_CURSES_WINDOW_H_
