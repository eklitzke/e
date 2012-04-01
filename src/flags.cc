// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./flags.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace {
po::variables_map vm_;

void PrintDescription(const po::options_description &desc,
                      FILE *f = stderr) {
  fprintf(f, "%s\n", boost::lexical_cast<std::string>(desc).c_str());
}
}

namespace e {
int ParseOptions(int argc, char **argv) {
  po::options_description help_desc("Help options");
  help_desc.add_options()
    ("debug,d", "run in debug mode")
    ("help,h", "produce this help message")
    ("help-module", po::value<std::string>(),
     "produce the help message for a given module")
    ("list-help-modules", "list all of the help modules");

  po::options_description scripting_desc("Scripting options");
  scripting_desc.add_options()
    ("skip-core", "skip loading any bundled \"core\" JS files")
    ("script,s", po::value<std::vector<std::string> >(),
     "script file(s) to load");

  po::options_description backend_desc("Backend options");
  backend_desc.add_options()
      ("really-do-nothing", "allow running without any JS scripts")
      ("file,f", po::value<std::vector<std::string> >(),
       "path to an input file to edit (any positional arguments will be "
       "assumed to also be input files, and that's the recommnded way to "
       "specify inputs)")
      ("without-boost-asio", "don't use boost::asio for the main loop");

  po::options_description all_desc("Allowed options");
  all_desc.add(help_desc).add(scripting_desc).add(backend_desc);

  po::options_description visible_desc;
  visible_desc.add(help_desc).add(scripting_desc);

  // all positional arguments are files
  po::positional_options_description p;
  p.add("file", -1);

  po::store(po::command_line_parser(argc, argv).
            options(all_desc).positional(p).run(), vm_);
  po::notify(vm_);

  if (vm_.count("help")) {
    printf("Usage: e [options] [file ...]\n");
    PrintDescription(visible_desc);
    return 0;
  }
  if (vm_.count("help-module")) {
    const std::string& s = vm_["help-module"].as<std::string>();
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
  if (vm_.count("list-help-modules")) {
    puts("backend");
    puts("help");
    puts("scripting");
    return 0;
  }

  // Generally a user should only be allowed to --skip-core if they specify
  // other scripts on the command line; if they try to skip this, make sure that
  // --really-do-nothing was specified.
  if (vm_.count("skip-core") &&
      !vm_.count("script") &&
      !vm_.count("really-do-nothing")) {
    printf(
        "Running with --skip-core and no --script arguments is probably a bad "
        "idea.\nIf you want to do this anyway, invoke with "
        "--really-do-nothing\n");
    return 1;
  }
  return NO_EXIT;
}

const po::variables_map& vm() {
  return vm_;
}
}
