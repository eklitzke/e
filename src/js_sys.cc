// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./js.h"
#include "./js_sys.h"

#include <unistd.h>
#include <signal.h>
#include <v8.h>

#ifdef USE_LINUX
#include <linux/limits.h>
#else
#define PATH_MAX 4096
#endif

using v8::Arguments;
using v8::HandleScope;
using v8::Integer;
using v8::ObjectTemplate;
using v8::String;
using v8::Undefined;

namespace e {
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

Handle<ObjectTemplate> GetSysTemplate() {
  HandleScope scope;
  Handle<ObjectTemplate> sys_templ = ObjectTemplate::New();
  sys_templ->Set(String::NewSymbol("chdir"),
                    FunctionTemplate::New(JSChdir), v8::ReadOnly);
  sys_templ->Set(String::NewSymbol("getcwd"),
                    FunctionTemplate::New(JSGetcwd), v8::ReadOnly);
  sys_templ->Set(String::NewSymbol("getpid"),
                    FunctionTemplate::New(JSGetpid), v8::ReadOnly);
  sys_templ->Set(String::NewSymbol("kill"),
                    FunctionTemplate::New(JSKill), v8::ReadOnly);
  return scope.Close(sys_templ);
}
}
