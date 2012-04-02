// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <boost/program_options.hpp>
#include <glog/logging.h>

#ifdef DISPOSE_v8
#include <v8.h>
#endif

#ifdef USE_LINUX
#include <sys/resource.h>
#endif

#include "./assert.h"
#include "./curses_window.h"
#include "./flags.h"
#include "./state.h"

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "main() entered";
  int return_code = e::ParseOptions(argc, argv);
  if (return_code != e::NO_EXIT) {
    return return_code;
  }
  const boost::program_options::variables_map &vm = e::vm();
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
#ifdef DISPOSE_v8
  v8::V8::Dispose();
#endif
#ifdef USE_LINUX
  rusage usage;
  ASSERT(getrusage(RUSAGE_SELF, &usage) == 0);
  LOG(INFO) << "max rss size: " << (usage.ru_maxrss / 1024) << " MB";
#endif  // USE_LINUX
  LOG(INFO) << "main() finishing with status 0";
  return 0;
}
