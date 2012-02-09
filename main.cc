// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <gflags/gflags.h>

#include "./buffer.h"
#include "./curses_window.h"
#include "./log.h"
#include "./state.h"

DEFINE_string(debug_log, "debug.log", "path to the debug log");
DEFINE_string(script, "main.js", "path to the JS script");

std::string load_script(void) {
  int script_fd = open(FLAGS_script.c_str(), O_RDONLY | O_CLOEXEC);
  if (script_fd == -1) {
    perror("open()");
    exit(1);
  }

  std::string script;
  char rd_buf[8000];
  while (true) {
    ssize_t bytes_read = read(script_fd, rd_buf, sizeof(rd_buf));
    if (bytes_read == -1) {
      perror("read()");
      close(script_fd);
      exit(1);
    } else if (bytes_read == 0) {
      break;
    }
    script.append(rd_buf, bytes_read);
  }
  return script;
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  // initialize logging
  e::log::init(FLAGS_debug_log);
  e::log::log_string("main.cc: starting up");

  std::string script = load_script();
  e::log::log_string("main.cc: read script");

  // create the world state
  e::State world(script);

  e::CursesWindow w(&world);
  w.loop();

  return 0;
}
