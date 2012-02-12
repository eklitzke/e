// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_STATE_H_
#define SRC_STATE_H_

#include <v8.h>

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
using v8::Value;

class State: public Embeddable {
 private:
  std::vector<Buffer *> buffers_;
  Buffer *active_buffer_;

  js::EventListener listener_;

 public:
  explicit State(const std::string &script);

  Buffer* get_active_buffer(void);
  std::vector<Buffer *>* get_buffers(void);

  // returns true if the mainloop should keep going, false otherwise
  bool handle_key(const KeyCode &);
};
}

#endif  // SRC_STATE_H_
