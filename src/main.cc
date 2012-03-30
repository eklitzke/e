// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <unistd.h>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>

#include "./assert.h"
#include "./curses_window.h"
#include "./flags.h"
#include "./list_environment.h"
#include "./state.h"

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  int return_code = e::ParseOptions(argc, argv);
  if (return_code != e::NO_EXIT) {
    return return_code;
  }

  const boost::program_options::variables_map &vm = e::vm();
  if (vm.count("list-environment")) {
    e::State state;
    state.LoadScript(false, e::ListEnvironment);
  } else {
    bool load_core = !vm.count("skip-core");
    std::vector<std::string> files;
    if (vm.count("file")) {
      files = vm["file"].as< std::vector<std::string> >();
    }
    std::vector<std::string> scripts;
    if (vm.count("script")) {
      scripts = vm["script"].as<std::vector<std::string> >();
    }
    e::CursesWindow window(load_core, scripts, files);
    window.Loop();
  }

  return 0;
}
