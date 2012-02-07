#include "buffer.h"
#include "curses_window.h"

int main(int argc, char **argv)
{
  if (argc == 1) {
    e::CursesWindow w;
    w.loop();
  } else {
    e::Buffer b(argv[1]);
    e::CursesWindow w(&b);
    w.loop();
  }
  return 0;
}
