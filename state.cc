// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <string>

#include "./state.h"

namespace e {

  State::State()
    :active_buffer_(new Buffer("*temp*")) {
    buffers_.push_back(active_buffer_);
  }

  State::State(Buffer *buf)
    :active_buffer_(buf) {
    buffers_.push_back(active_buffer_);
  }

  State::~State() {
  }

  Buffer *
  State::get_active_buffer(void) {
    return active_buffer_;
  }

  std::vector<Buffer *> *
  State::get_buffers(void) {
    return &buffers_;
  }

  bool
  State::handle_key(const KeyCode &k) {
    if (k.is_ascii() && k.get_char() == 'q') {
      return false;
    }
    return true;
  }
}
