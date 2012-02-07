#include <string>

#include "state.h"

namespace e {

  State::State()
    :active_buffer_(new Buffer("*temp*"))
  {
    buffers_.push_back(active_buffer_);
  }

  State::State(Buffer *b)
    :active_buffer_(b)
  {
    buffers_.push_back(active_buffer_);
  }

  State::~State()
  {
  }

  Buffer *
  State::get_active_buffer(void)
  {
    return active_buffer_;
  }

  std::vector<Buffer *> *
  State::get_buffers(void)
  {
    return &buffers_;
  }

}
