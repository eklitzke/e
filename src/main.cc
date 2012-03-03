// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <stdio.h>
#include <unistd.h>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>

#include "./curses_window.h"
#include "./list_api.h"
#include "./state.h"

namespace po = boost::program_options;

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);

  std::string script = "js/core.js";
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "produce help message")
      ("script,s", po::value<std::string>(&script), "the JS script to load")
      ("list-api", "list the available JS API");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    fprintf(stdout, "%s\n", boost::lexical_cast<std::string>(desc).c_str());
    return 1;
  }

  DLOG(INFO) << "started up";

  if (access(script.c_str(), R_OK) != 0) {
    fprintf(stdout, "cannot access script \"%s\"\n", script.c_str());
    return 1;
  }

  if (vm.count("list-api")) {
    e::State state;
    state.LoadScript(false, e::ListAPI);
  } else {
    e::CursesWindow window(script);
    window.Loop();
  }

  return 0;
}
