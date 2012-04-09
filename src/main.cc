// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <boost/program_options.hpp>
#include <v8.h>

#ifdef USE_LINUX
#include <sys/resource.h>
#endif

#include "./assert.h"
#include "./curses_window.h"
#include "./flags.h"
#include "./logging.h"
#include "./state.h"

int main(int argc, char **argv) {
  e::InitLogging("editor.log");
  int return_code = e::ParseOptions(argc, argv);
  if (return_code != e::NO_EXIT) {
    return return_code;
  }
  const boost::program_options::variables_map &vm = e::vm();
  if (vm.count("debug")) {
    e::SetDefaultLogLevel(e::DBG);
    e::LOG(e::DBG, "debug logging turned on");
  }
  std::vector<std::string> files;
  if (vm.count("file")) {
    files = vm["file"].as< std::vector<std::string> >();
  }
  std::vector<std::string> scripts;
  if (vm.count("script")) {
    scripts = vm["script"].as<std::vector<std::string> >();
  }
  e::CursesWindow window(scripts, files);
  window.Loop();
  v8::V8::Dispose();  // to assist heap checking
#ifdef USE_LINUX
  rusage usage;
  ASSERT(getrusage(RUSAGE_SELF, &usage) == 0);
  e::LOG(e::INFO, "max rss size: %d MB", usage.ru_maxrss / 1024);
#endif  // USE_LINUX
  e::LOG(e::INFO, "main() finishing with status 0");
  return 0;
}
