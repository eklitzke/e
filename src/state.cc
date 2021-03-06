// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./state.h"

#include <boost/asio.hpp>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <v8.h>

#include <functional>
#include <string>

#if OPTIMIZED_BUILD
#include "./bundled_core.h"
#endif
#include "./embeddable.h"
#include "./flags.h"
#include "./io_service.h"
#include "./js.h"
#include "./logging.h"
#include "./module_decl.h"
#include "./timer.h"

namespace e {

using v8::Arguments;
using v8::Array;
using v8::Boolean;
using v8::Context;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Function;
using v8::FunctionTemplate;
using v8::Integer;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::Persistent;
using v8::Script;
using v8::String;
using v8::Template;
using v8::TryCatch;
using v8::Undefined;
using v8::Value;

// Path to the user-specific configuration script; this is relative to the
// user's home directory.
const char *kInitFile = ".e.js";

namespace {
bool keep_going = true;

// @class: world
// @description: The representation of the world's state.

// @method: stopLoop
// @description: Stops the event loop.
Handle<Value> JSStopLoop(const Arguments& args) {
  keep_going = false;
  io_service.stop();
  return Undefined();
}

// @method: addEventListener
// @param[type]: #string the type of event to listen for
// @param[listener]: #function the callback function to invoke
// @param[useCapture]: #bool run the listener in capture mode (optional,
//                     defaults to `false`)
// @description: Adds an event listener to the world. This follows the usual
//               browser-based `addEventListener` interface.
Handle<Value> JSAddEventListener(const Arguments& args) {
  CHECK_ARGS(2);
  GET_SELF(State);

  Local<String> event_name = args[0]->ToString();

  Local<Value> callback = args[1];
  if (!callback->IsObject()) {
    return Undefined();
  }

  bool use_capture = true;
  if (args.Length() >= 3) {
    use_capture = args[2]->BooleanValue();
  }

  Local<Object> obj = callback->ToObject();
  self->GetListener()->Add(
      js::ValueToString(event_name), obj, use_capture);

  return Undefined();
}
}

State::State(const std::vector<std::string> &scripts,
             const std::vector<std::string> &args)
    :scripts_(scripts), args_(args) {
  Buffer *scratch_buffer = new Buffer("*temp*");
  buffers_.push_back(scratch_buffer);
}

State::~State() {
  for (auto it = buffers_.begin(); it != buffers_.end(); ++it) {
    delete *it;
  }
}

void State::Run(std::function<void()> then) {
  HandleScope scope;
  Local<ObjectTemplate> global = ObjectTemplate::New();
  AddJsToGlobalNamespace(global);  // add log(), require(), etc.
  AddTimersToGlobalNamespace(global);  // add setTimeout() and co.

  Handle<ObjectTemplate> world_templ = ObjectTemplate::New();
  world_templ->SetInternalFieldCount(1);
  js::AddTemplateFunction(world_templ, "addEventListener", JSAddEventListener);
  js::AddTemplateFunction(world_templ, "stopLoop", JSStopLoop);

  Persistent<Context> context = Context::New(nullptr, global);
  Context::Scope context_scope(context);

  Local<Object> world = world_templ->NewInstance();
  world->SetInternalField(0, External::New(this));
  world->Set(String::NewSymbol("buffer"), buffers_[0]->ToScript(),
             v8::ReadOnly);
  context->Global()->Set(String::NewSymbol("world"), world, v8::ReadOnly);
  context->Global()->Set(String::NewSymbol("debug"),
                         Boolean::New(vm.count("debug")), v8::ReadOnly);

  // export the editor's argv array to JS (as `args')
  Local<Array> args = Array::New(args_.size());
  world->Set(String::NewSymbol("args"), args, v8::ReadOnly);
  for (auto it = args_.begin(); it != args_.end(); ++it) {
    args->Set(it - args_.begin(), String::New(it->c_str(), it->size()));
  }

  // initialize all of the builtin modules (e.g. curses, errno, sys)
  InitializeBuiltinModules();

  bool bail = false;

  // Load the core script; this should be known to be good and not throw
  // exceptions.
#if OPTIMIZED_BUILD
  if (vm.count("debug")) {
    EnsureCoreScript();
  } else if (!vm.count("skip-core")) {
    LOG(INFO, "loading builtin core.js");
    TryCatch trycatch;
    Local<Script> script = GetCoreScript();
    HandleError(trycatch);
    script->Run();
    HandleError(trycatch);
    LOG(INFO, "finished loading builtin core.js");
  }
#else
  EnsureCoreScript();
#endif

  // add the init file
  if (vm.count("no-init-file") == 0) {
    uid_t user = getuid();
    passwd pwent;
    passwd *temp_pwent;
    long pwbuf_len = sysconf(_SC_GETPW_R_SIZE_MAX);  // NOLINT
    std::unique_ptr<char[]> pwbuf(new char[static_cast<size_t>(pwbuf_len)]);
    ASSERT(getpwuid_r(user, &pwent, pwbuf.get(), pwbuf_len, &temp_pwent) == 0);
    ASSERT(temp_pwent != nullptr);
    ASSERT(&pwent == temp_pwent);
    std::string rc_path(pwent.pw_dir);
    rc_path += "/";
    rc_path += kInitFile;
    if (access(rc_path.c_str(), R_OK) == 0) {
      scripts_.push_back(rc_path);
    } else {
      LOG(DBG, "failed to find (or could not access) init file \"%s\"",
          rc_path.c_str());
    }
  }

  // sequentially load any other scripts
  for (auto it = scripts_.cbegin(); it != scripts_.cend(); ++it) {
    LOG(INFO, "loading additional script \"%s\"", it->c_str());
    TryCatch trycatch;
    Handle<String> source = js::ReadFile(*it);
    Handle<Script> scr = Script::New(
        source, String::New(it->c_str(), it->size()));
    HandleError(trycatch);
    scr->Run();
    HandleError(trycatch);
    LOG(DBG, "finished loading additional script \"%s\"", it->c_str());
  }

  // run the callback
  if (!bail && keep_going) {
    then();
  }
  CancelAllTimers();
  context.Dispose();
}

void State::EnsureCoreScript() {
  bool seen_core = false;
  for (auto it = scripts_.cbegin(); it != scripts_.cend(); ++it) {
    if (*it == "js/core.js") {
      seen_core = true;
      break;
    }
  }
  if (!seen_core) {
    scripts_.insert(scripts_.begin(), "js/core.js");
  }
}

bool State::HandleKey(KeyCode *k) {
  HandleScope scope;

  std::vector<Handle<Value> > args;
  args.push_back(k->ToScript());
  TryCatch trycatch;
  listener_.Dispatch("keypress", args);
  HandleError(trycatch);
  listener_.Dispatch("after_keypress", args);
  HandleError(trycatch);

  return keep_going;
}
}
