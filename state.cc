#include <string>

#include "state.h"
#include "term.h"

namespace e {

  State::State()
    :active_buffer_(NULL)
  {
  }

  State::State(Buffer *b)
    :active_buffer_(b)
  {
    buffers_.push_back(b);
  }

  State::~State()
  {
  }

}
