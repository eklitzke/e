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

  // @class: signal
  // @description: A collection of various signal constants

  // @accessor: NSIG
  NEW_INTEGER(signal_templ, NSIG);

  // @accessor: SIGABRT
  NEW_INTEGER(signal_templ, SIGABRT);

  // @accessor: SIGALRM
  NEW_INTEGER(signal_templ, SIGALRM);

  // @accessor: SIGBUS
  NEW_INTEGER(signal_templ, SIGBUS);

  // @accessor: SIGCHLD
  NEW_INTEGER(signal_templ, SIGCHLD);

  // @accessor: SIGCLD
  NEW_INTEGER(signal_templ, SIGCLD);

  // @accessor: SIGCONT
  NEW_INTEGER(signal_templ, SIGCONT);

  // @accessor: SIGFPE
  NEW_INTEGER(signal_templ, SIGFPE);

  // @accessor: SIGHUP
  NEW_INTEGER(signal_templ, SIGHUP);

  // @accessor: SIGILL
  NEW_INTEGER(signal_templ, SIGILL);

  // @accessor: SIGINT
  NEW_INTEGER(signal_templ, SIGINT);

  // @accessor: SIGIO
  NEW_INTEGER(signal_templ, SIGIO);

  // @accessor: SIGIOT
  NEW_INTEGER(signal_templ, SIGIOT);

  // @accessor: SIGKILL
  NEW_INTEGER(signal_templ, SIGKILL);

  // @accessor: SIGPIPE
  NEW_INTEGER(signal_templ, SIGPIPE);

  // @accessor: SIGPOLL
  NEW_INTEGER(signal_templ, SIGPOLL);

  // @accessor: SIGPROF
  NEW_INTEGER(signal_templ, SIGPROF);

  // @accessor: SIGPWR
  NEW_INTEGER(signal_templ, SIGPWR);

  // @accessor: SIGQUIT
  NEW_INTEGER(signal_templ, SIGQUIT);

  // @accessor: SIGRTMAX
  NEW_INTEGER(signal_templ, SIGRTMAX);

  // @accessor: SIGRTMIN
  NEW_INTEGER(signal_templ, SIGRTMIN);

  // @accessor: SIGSEGV
  NEW_INTEGER(signal_templ, SIGSEGV);

  // @accessor: SIGSTOP
  NEW_INTEGER(signal_templ, SIGSTOP);

  // @accessor: SIGSYS
  NEW_INTEGER(signal_templ, SIGSYS);

  // @accessor: SIGTERM
  NEW_INTEGER(signal_templ, SIGTERM);

  // @accessor: SIGTRAP
  NEW_INTEGER(signal_templ, SIGTRAP);

  // @accessor: SIGTSTP
  NEW_INTEGER(signal_templ, SIGTSTP);

  // @accessor: SIGTTIN
  NEW_INTEGER(signal_templ, SIGTTIN);

  // @accessor: SIGTTOU
  NEW_INTEGER(signal_templ, SIGTTOU);

  // @accessor: SIGURG
  NEW_INTEGER(signal_templ, SIGURG);

  // @accessor: SIGUSR1
  NEW_INTEGER(signal_templ, SIGUSR1);

  // @accessor: SIGUSR2
  NEW_INTEGER(signal_templ, SIGUSR2);

  // @accessor: SIGVTALRM
  NEW_INTEGER(signal_templ, SIGVTALRM);

  // @accessor: SIGWINCH
  NEW_INTEGER(signal_templ, SIGWINCH);

  // @accessor: SIGXCPU
  NEW_INTEGER(signal_templ, SIGXCPU);

  // @accessor: SIGXFSZ
  NEW_INTEGER(signal_templ, SIGXFSZ);

  return scope.Close(signal_templ);
}
}
