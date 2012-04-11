// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// This file is the main entry point for the editor (it's what loads all of the
// JavaScript, and is run by the main loop).

#ifndef SRC_STATE_H_
#define SRC_STATE_H_

#include <v8.h>

#include <functional>
#include <string>
#include <vector>

#include "./buffer.h"
#include "./event_listener.h"
#include "./keycode.h"

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
  void Run(std::function<void()> callback);

  EventListener* GetListener(void) { return &listener_; }

  // returns true if the mainloop should keep going, false otherwise
  bool HandleKey(KeyCode *k);

 private:
  std::vector<std::string> scripts_;
  std::vector<std::string> args_;

  std::vector<Buffer*> buffers_;
  EventListener listener_;

  // Ensure that the js/core.js script is in the list of loaded scripts; if it's
  // not, insert it at the front of the scripts_ list.
  void EnsureCoreScript();
};
}

#endif  // SRC_STATE_H_
