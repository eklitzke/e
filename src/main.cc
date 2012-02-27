// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <unistd.h>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>

#include "./buffer.h"
#include "./termios_window.h"
#include "./state.h"

namespace po = boost::program_options;

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);

  std::string script_name = "js/base.js";
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "produce help message")
      ("script,s", po::value<std::string>(&script_name),
       "The JS script to load");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    fprintf(stdout, "%s\n", boost::lexical_cast<std::string>(desc).c_str());
    return 1;
  }

  DLOG(INFO) << "started up";

  if (access(script_name.c_str(), R_OK) != 0) {
    fprintf(stdout, "cannot access script \"%s\"\n", script_name.c_str());
    return 1;
  }

  e::TermiosWindow window(script_name);
  window.Loop();

  return 0;
}
