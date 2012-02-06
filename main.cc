#include <iostream>

#include <curses.h>
#include <unistd.h>

#include "curses_window.h"

int main(int argc, char **argv)
{
  e::CursesWindow w;
  w.loop();
  return 0;
}
