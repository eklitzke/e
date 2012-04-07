// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// This file is the main entry point for the editor (it's what loads all of the
// JavaScript, and is run by the main loop).

#ifndef SRC_STATE_H_
#define SRC_STATE_H_

#include <v8.h>

#include <boost/function.hpp>

#include <string>
#include <vector>

#include "./buffer.h"
#include "./event_listener.h"
#include "./keycode.h"

using v8::Arguments;
using v8::Context;
using v8::Function;
using v8::Handle;
using v8::Persistent;
using v8::Script;
using v8::Value;

namespace e {

class State {
 public:
  State() {}
  explicit State(const std::vector<std::string> &scripts,
                 const std::vector<std::string> &args);
  ~State();

  // This is the main entry point that runs the program. It initializes all of
  // the JavaScript stuft, and then runs the callback function. After the
  // callback function completes, everything will be cleaned up.
  //
  // It's expected that callback will implement some sort of loop (i.e. the I/O
  // loop that drives the editor), although strictly speaking this is not
  // necessary.
  void Run(boost::function<void(Persistent<Context>)> callback);

  EventListener* GetListener(void) { return &listener_; }

  // returns true if the mainloop should keep going, false otherwise
  bool HandleKey(KeyCode *k);

  v8::Persistent<v8::Object> callback_o;

 private:
  std::vector<std::string> scripts_;
  std::vector<std::string> args_;

  std::vector<Buffer*> buffers_;
  EventListener listener_;
};
}

#endif  // SRC_STATE_H_
