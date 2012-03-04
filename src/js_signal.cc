// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./js.h"
#include "./js_signal.h"

#include <v8.h>
#include <csignal>

using v8::Handle;
using v8::HandleScope;
using v8::ObjectTemplate;

namespace e {
Handle<ObjectTemplate> GetSignalTemplate() {
  HandleScope scope;
  Handle<ObjectTemplate> signal_templ = ObjectTemplate::New();
  NEW_INTEGER(signal_templ, NSIG);
  NEW_INTEGER(signal_templ, SIGABRT);
  NEW_INTEGER(signal_templ, SIGALRM);
  NEW_INTEGER(signal_templ, SIGBUS);
  NEW_INTEGER(signal_templ, SIGCHLD);
  NEW_INTEGER(signal_templ, SIGCLD);
  NEW_INTEGER(signal_templ, SIGCONT);
  NEW_INTEGER(signal_templ, SIGFPE);
  NEW_INTEGER(signal_templ, SIGHUP);
  NEW_INTEGER(signal_templ, SIGILL);
  NEW_INTEGER(signal_templ, SIGINT);
  NEW_INTEGER(signal_templ, SIGIO);
  NEW_INTEGER(signal_templ, SIGIOT);
  NEW_INTEGER(signal_templ, SIGKILL);
  NEW_INTEGER(signal_templ, SIGPIPE);
  NEW_INTEGER(signal_templ, SIGPOLL);
  NEW_INTEGER(signal_templ, SIGPROF);
  NEW_INTEGER(signal_templ, SIGPWR);
  NEW_INTEGER(signal_templ, SIGQUIT);
  NEW_INTEGER(signal_templ, SIGRTMAX);
  NEW_INTEGER(signal_templ, SIGRTMIN);
  NEW_INTEGER(signal_templ, SIGSEGV);
  NEW_INTEGER(signal_templ, SIGSTOP);
  NEW_INTEGER(signal_templ, SIGSYS);
  NEW_INTEGER(signal_templ, SIGTERM);
  NEW_INTEGER(signal_templ, SIGTRAP);
  NEW_INTEGER(signal_templ, SIGTSTP);
  NEW_INTEGER(signal_templ, SIGTTIN);
  NEW_INTEGER(signal_templ, SIGTTOU);
  NEW_INTEGER(signal_templ, SIGURG);
  NEW_INTEGER(signal_templ, SIGUSR1);
  NEW_INTEGER(signal_templ, SIGUSR2);
  NEW_INTEGER(signal_templ, SIGVTALRM);
  NEW_INTEGER(signal_templ, SIGWINCH);
  NEW_INTEGER(signal_templ, SIGXCPU);
  NEW_INTEGER(signal_templ, SIGXFSZ);
  return scope.Close(signal_templ);
}
}
