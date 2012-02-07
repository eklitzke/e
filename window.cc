#include "buffer.h"
#include "window.h"

namespace e {

  Window::Window(Buffer *b)
    :state_(b)
  {
  }

}
