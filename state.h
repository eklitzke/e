// -*- C++ -*-

#include <string>
#include <vector>

#include "buffer.h"

namespace e {

  class State
  {
  private:
    std::vector<Buffer *> buffers_;
    Buffer *active_buffer_;

  public:
    State();
    State(Buffer *);
    ~State();

    Buffer* get_active_buffer(void);
    std::vector<Buffer *>* get_buffers(void);
  };
}
