// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./js.h"
#include "./js_signal.h"

#include <v8.h>
#include <csignal>


using v8::Handle;
using v8::HandleScope;
using v8::ObjectTemplate;

namespace e {
namespace js_signal {
bool Build(Handle<Object> obj) {
  HandleScope scope;

  // @class: signal
  // @description: A collection of various signal constants

  // @accessor: NSIG
  NEW_INTEGER(obj, NSIG);

  // @accessor: SIGABRT
  NEW_INTEGER(obj, SIGABRT);

  // @accessor: SIGALRM
  NEW_INTEGER(obj, SIGALRM);

  // @accessor: SIGBUS
  NEW_INTEGER(obj, SIGBUS);

  // @accessor: SIGCHLD
  NEW_INTEGER(obj, SIGCHLD);

  // @accessor: SIGCLD
  NEW_INTEGER(obj, SIGCLD);

  // @accessor: SIGCONT
  NEW_INTEGER(obj, SIGCONT);

  // @accessor: SIGFPE
  NEW_INTEGER(obj, SIGFPE);

  // @accessor: SIGHUP
  NEW_INTEGER(obj, SIGHUP);

  // @accessor: SIGILL
  NEW_INTEGER(obj, SIGILL);

  // @accessor: SIGINT
  NEW_INTEGER(obj, SIGINT);

  // @accessor: SIGIO
  NEW_INTEGER(obj, SIGIO);

  // @accessor: SIGIOT
  NEW_INTEGER(obj, SIGIOT);

  // @accessor: SIGKILL
  NEW_INTEGER(obj, SIGKILL);

  // @accessor: SIGPIPE
  NEW_INTEGER(obj, SIGPIPE);

  // @accessor: SIGPOLL
  NEW_INTEGER(obj, SIGPOLL);

  // @accessor: SIGPROF
  NEW_INTEGER(obj, SIGPROF);

  // @accessor: SIGPWR
  NEW_INTEGER(obj, SIGPWR);

  // @accessor: SIGQUIT
  NEW_INTEGER(obj, SIGQUIT);

  // @accessor: SIGRTMAX
  NEW_INTEGER(obj, SIGRTMAX);

  // @accessor: SIGRTMIN
  NEW_INTEGER(obj, SIGRTMIN);

  // @accessor: SIGSEGV
  NEW_INTEGER(obj, SIGSEGV);

  // @accessor: SIGSTOP
  NEW_INTEGER(obj, SIGSTOP);

  // @accessor: SIGSYS
  NEW_INTEGER(obj, SIGSYS);

  // @accessor: SIGTERM
  NEW_INTEGER(obj, SIGTERM);

  // @accessor: SIGTRAP
  NEW_INTEGER(obj, SIGTRAP);

  // @accessor: SIGTSTP
  NEW_INTEGER(obj, SIGTSTP);

  // @accessor: SIGTTIN
  NEW_INTEGER(obj, SIGTTIN);

  // @accessor: SIGTTOU
  NEW_INTEGER(obj, SIGTTOU);

  // @accessor: SIGURG
  NEW_INTEGER(obj, SIGURG);

  // @accessor: SIGUSR1
  NEW_INTEGER(obj, SIGUSR1);

  // @accessor: SIGUSR2
  NEW_INTEGER(obj, SIGUSR2);

  // @accessor: SIGVTALRM
  NEW_INTEGER(obj, SIGVTALRM);

  // @accessor: SIGWINCH
  NEW_INTEGER(obj, SIGWINCH);

  // @accessor: SIGXCPU
  NEW_INTEGER(obj, SIGXCPU);

  // @accessor: SIGXFSZ
  NEW_INTEGER(obj, SIGXFSZ);

  return true;
}
}
}
