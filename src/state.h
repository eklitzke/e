// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_STATE_H_
#define SRC_STATE_H_

#include <v8.h>

#include <boost/function.hpp>

#include <string>
#include <vector>

#include "./buffer.h"
#include "./embeddable.h"
#include "./js.h"
#include "./keycode.h"

namespace e {

using v8::Arguments;
using v8::Context;
using v8::Function;
using v8::Handle;
using v8::Persistent;
using v8::Script;
using v8::Value;

class State: public Embeddable {
 public:
  explicit State(const std::string &script_name);
  void RunScript(boost::function<void ()>);

  js::EventListener* GetListener(void) { return &listener_; }

  Buffer* GetActiveBuffer(void);
  std::vector<Buffer *>* GetBuffers(void);

  // returns true if the mainloop should keep going, false otherwise
  bool HandleKey(KeyCode *);

  v8::Persistent<v8::Object> callback_o;
 private:
  std::vector<Buffer *> buffers_;
  Buffer *active_buffer_;
  js::EventListener listener_;
  std::string script_name_;


};
}

#endif  // SRC_STATE_H_
