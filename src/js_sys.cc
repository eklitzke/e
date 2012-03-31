// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./js_sys.h"

#include <v8.h>
#include <unistd.h>

#ifdef USE_LINUX
#include <linux/limits.h>
#else
#define PATH_MAX 4096
#endif

#include <csignal>

#include "./js.h"
#include "./module.h"

using v8::Arguments;
using v8::HandleScope;
using v8::Integer;
using v8::ObjectTemplate;
using v8::String;
using v8::Undefined;

namespace {
// @class: sys
// @description: Various low-level system routines.

// @method: chdir
// @param[path]: #string the directory to change to
// @description: An implementation of `chdir(2)`.
Handle<Value> JSChdir(const Arguments& args) {
  HandleScope scope;
  if (args.Length() < 1) {
    return Undefined();
  }
  Local<Value> path = args[0];
  int ret = chdir(*String::AsciiValue(path));
  return scope.Close(Integer::New(ret));
}

// @method: getcwd
// @description: Returns the current working directory.
Handle<Value> JSGetcwd(const Arguments& args) {
  HandleScope scope;
  static char buf[PATH_MAX];
  getcwd(buf, sizeof(buf));
  return scope.Close(String::New(buf));
}

// @method: getpid
// @description: Returns the PID of the current process as an integer.
Handle<Value> JSGetpid(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Integer::New(static_cast<int>(getpid())));
}

// @method: kill
// @param[pid]: #int the process to signal
// @param[sig]: #int the signal to send
// @description: An implementation of `kill(2)`.
Handle<Value> JSKill(const Arguments& args) {
  HandleScope scope;
  if (args.Length() < 2) {
    return Undefined();
  }
  uint32_t pid = args[0]->Uint32Value();
  uint32_t signal = args[1]->Uint32Value();
  int ret = kill(static_cast<pid_t>(pid), static_cast<int>(signal));
  return scope.Close(Integer::New(ret));
}
}

namespace e {
namespace js_sys {
bool Build(Handle<Object> obj) {
  HandleScope scope;
  AddFunction(obj, "chdir", JSChdir);
  AddFunction(obj, "getcwd", JSGetcwd);
  AddFunction(obj, "getpid", JSGetpid);
  AddFunction(obj, "kill", JSKill);
  return true;
}
}
}
