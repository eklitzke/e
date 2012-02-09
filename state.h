// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef STATE_H_
#define STATE_H_

#include <v8.h>

#include <string>
#include <vector>

#include "./buffer.h"
#include "./keycode.h"

namespace e {
  class State {
  private:
    std::vector<Buffer *> buffers_;
    Buffer *active_buffer_;

    v8::Persistent<v8::Context> context_;
    v8::Persistent<v8::Function> onkeypress_;

  public:
    explicit State(const std::string &script);
    ~State();

    Buffer* get_active_buffer(void);
    std::vector<Buffer *>* get_buffers(void);

    // returns true if the mainloop should keep going, false otherwise
    bool handle_key(const KeyCode &);
  };
}

#endif  // STATE_H_
