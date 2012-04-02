// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <boost/program_options.hpp>
#include <glog/logging.h>

#ifdef USE_LINUX
#include <sys/resource.h>
#include <sys/time.h>
#endif

#include "./assert.h"
#include "./curses_window.h"
#include "./flags.h"
#include "./state.h"

#ifdef USE_LINUX
double get_millis(const timeval &t) {
  return (t.tv_sec * 1000000 + t.tv_usec) / 1000.0;
}
#endif

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
#ifdef USE_LINUX
  rusage usage;
  ASSERT(getrusage(RUSAGE_SELF, &usage) == 0);
  LOG(INFO) << "";
  LOG(INFO) << "Final Resource Utilization";
  LOG(INFO) << "--------------------------";
  LOG(INFO) << "utime: " << get_millis(usage.ru_utime) << " ms";
  LOG(INFO) << "stime: " << get_millis(usage.ru_stime) << " ms";
  LOG(INFO) << "maxrss: " << (usage.ru_maxrss / 1024.0) << " MB";
  LOG(INFO) << "";
#endif  // USE_LINUX
  LOG(INFO) << "main() finishing with status 0";
  return 0;
}
