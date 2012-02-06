#include <iostream>

#include <curses.h>
#include <unistd.h>

#include "buffer.h"
#include "state.h"

int linenum = 0;

using namespace e;

int main(int argc, char **argv)
{
  State *s;
  if (argc >= 2) {
	Buffer b("tmp", argv[1]);
	s = new State(&b);
  } else {
	s = new State();
  }

  s->redraw();
  getch();

  delete s;

  return 0;
}
