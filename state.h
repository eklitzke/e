// -*- C++ -*-

#include <string>
#include <vector>

#include "buffer.h"
#include "statusbar.h"

namespace e {

  class State
  {
  private:
    std::vector<Buffer *> buffers_;
    Buffer *active_buffer_;
    std::string status_;
    StatusBar statusbar_;

    void update_status();

  public:
    State();
    State(Buffer *);
    ~State();
  };
}
