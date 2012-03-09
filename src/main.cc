// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <stdio.h>
#include <unistd.h>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>

#include "./curses_window.h"
#include "./list_environment.h"
#include "./state.h"

namespace po = boost::program_options;

void PrintDescription(const po::options_description &desc,
					  FILE *f = stderr) {
  fprintf(f, "%s\n", boost::lexical_cast<std::string>(desc).c_str());
}

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);

  po::options_description help_desc("Help options");
  help_desc.add_options()
	("help,h", "produce help message")
	("help-module", po::value<std::string>(),
	 "produce help message for a given module");

  po::options_description scripting_desc("Scripting options");
  scripting_desc.add_options()
	("skip-core", "skip loading any bundled \"core\" JS files")
	("script,s", po::value<std::vector<std::string> >(),
	 "script file(s) to load")
	("list-environment", "list the default JS environment");

  po::options_description backend_desc("Backend options");
  backend_desc.add_options()
      ("really-do-nothing", "allow running without any JS scripts")
      ("file,f", po::value<std::vector<std::string> >(),
       "path to an input file to edit (any positional arguments will be "
	   "assumed to also be input files, and that's the recommnded way to "
	   "specify inputs)");
	  
  po::options_description all_desc("Allowed options");
  all_desc.add(help_desc).add(scripting_desc).add(backend_desc);

  po::options_description visible_desc("Allowed options");
  visible_desc.add(help_desc).add(scripting_desc);

  // all positional arguments are files
  po::positional_options_description p;
  p.add("file", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).
			options(all_desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
	printf("Usage: e [options] [file ...]\n\n");
	PrintDescription(visible_desc);
	return 0;
  }
  if (vm.count("help-module")) {
	const std::string& s = vm["help-module"].as<std::string>();
	if (s == "help") {
	  PrintDescription(help_desc);
	} else if (s == "scripting") {
	  PrintDescription(scripting_desc);
	} else if (s == "backend") {
	  PrintDescription(backend_desc);
	} else {
	  printf("Unknown module '%s'\n", s.c_str());
	  return 1;
	}
	return 0;
  }

  // Generally a user should only be allowed to --skip-core if they specify
  // other scripts on the command line; if they try to skip this, make sure that
  // --really-do-nothing was specified.
  if (vm.count("skip-core") &&
      !vm.count("script") &&
      !vm.count("really-do-nothing")) {
    printf(
        "Running with --skip-core and no --script arguments is probably a bad "
        "idea.\nIf you want to do this anyway, invoke with "
        "--really-do-nothing\n");
    return 1;
  }

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
