// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <unistd.h>

#include <gflags/gflags.h>

#include "./buffer.h"
#include "./curses_window.h"
#include "./log.h"
#include "./state.h"

DEFINE_string(debug_log, "debug.log", "path to the debug log");
DEFINE_string(script, "main.js", "path to the JS script");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  // initialize logging
  e::log::init(FLAGS_debug_log);
  e::log::log_string("main.cc: starting up");

  if (access(FLAGS_script.c_str(), R_OK) != 0) {
    fprintf(stderr, "cannot access script \"%s\"\n", FLAGS_script.c_str());
    return 1;
  }

  // create the world state
  e::State world(FLAGS_script);

  e::CursesWindow w(&world);
  w.loop();

  return 0;
}
