// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_CURSES_WINDOW_H_
#define SRC_CURSES_WINDOW_H_

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

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
