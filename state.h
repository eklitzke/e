// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef STATE_H_
#define STATE_H_

#include <string>
#include <vector>

#include "./buffer.h"

namespace e {
  class State {
  private:
    std::vector<Buffer *> buffers_;
    Buffer *active_buffer_;

  public:
    State();
    explicit State(Buffer *buf);
    ~State();

    Buffer* get_active_buffer(void);
    std::vector<Buffer *>* get_buffers(void);
  };
}

#endif  // STATE_H_
