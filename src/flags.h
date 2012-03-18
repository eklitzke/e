// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_FLAGS_H_
#define SRC_FLAGS_H_

#include <boost/program_options.hpp>

namespace e {
enum {
  // sentinel to not use the return value from ParseOptions()
  NO_EXIT = 42
};

// parse argc and argv
int ParseOptions(int argc, char **argv);

// get the variables map
const boost::program_options::variables_map& vm();
}

#endif  // SRC_FLAGS_H_
