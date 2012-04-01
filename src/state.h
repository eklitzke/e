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
  void LoadScript(bool, boost::function<void(Persistent<Context>)>);

  EventListener* GetListener(void) { return &listener_; }

  std::vector<Buffer*>* GetBuffers(void);

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
