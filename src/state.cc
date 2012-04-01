// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./state.h"

#include <v8.h>
#include <glog/logging.h>

#include <string>

#include "./bundled_core.h"
#include "./embeddable.h"
#include "./flags.h"
#include "./js.h"
#include "./module_decl.h"

namespace e {

using v8::Arguments;
using v8::Array;
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

namespace {
bool keep_going = true;

// @class: world
// @description: The representation of the world's state.

// @method: addEventListener
// @param[type]: #string the type of event to listen for
// @param[listener]: #function the callback function to invoke
// @param[useCapture]: #bool run the listener in capture mode (optional,
//                     defaults to `false`)
// @description: Adds an event listener to the world. This follows the usual
//               browser-based `addEventListener` interface.

// @method: stopLoop
// @description: Stops the event loop.
Handle<Value> JSStopLoop(const Arguments& args) {
  keep_going = false;
  return Undefined();
}

Handle<Value> AddEventListener(const Arguments& args) {
  CHECK_ARGS(2);
  GET_SELF(State);

  Local<String> event_name = args[0]->ToString();

  Handle<Value> callback = args[1];
  if (!callback->IsObject()) {
    return Undefined();
  }
  self->callback_o = Persistent<Object>::New(callback->ToObject());

  bool use_capture = true;
  if (args.Length() >= 3) {
    use_capture = args[2]->BooleanValue();
  }

  self->GetListener()->Add(
      js::ValueToString(event_name), self->callback_o, use_capture);

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

void State::LoadScript(bool run,
                       boost::function<void(Persistent<Context>)> then) {
  HandleScope scope;
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  global->Set(String::NewSymbol("assert"),
              FunctionTemplate::New(js::JSAssert), v8::ReadOnly);
  global->Set(String::NewSymbol("log"),
              FunctionTemplate::New(js::JSLog), v8::ReadOnly);
  global->Set(String::NewSymbol("require"),
              FunctionTemplate::New(js::JSRequire), v8::ReadOnly);

  Handle<ObjectTemplate> world_templ = ObjectTemplate::New();
  world_templ->SetInternalFieldCount(1);
  js::AddTemplateFunction(world_templ, "addEventListener", AddEventListener);
  js::AddTemplateFunction(world_templ, "stopLoop", JSStopLoop);

  Persistent<Context> context = InitializeContext(global);
  Context::Scope context_scope(context);

  Local<Object> world = world_templ->NewInstance();
  world->SetInternalField(0, External::New(this));
  world->Set(String::NewSymbol("buffer"), buffers_[0]->ToScript(),
             v8::ReadOnly);
  context->Global()->Set(String::NewSymbol("world"), world, v8::ReadOnly);

  // export the editor's argv array to JS (as `args')
  Local<Array> args = Array::New(args_.size());
  world->Set(String::NewSymbol("args"), args, v8::ReadOnly);
  for (auto it = args_.begin(); it != args_.end(); ++it) {
    args->Set(it - args_.begin(), String::New(it->c_str(), it->size()));
  }

  // initialize all of the builtin modules (e.g. curses, errno, sys)
  InitializeBuiltinModules();

  bool bail = false;
  if (run) {
    // Load the core script; this should be known to be good and not throw
    // exceptions.
    if (vm().count("debug")) {
      scripts_.insert(scripts_.begin(), "js/core.js");
    } else if (!vm().count("skip-core")) {
      LOG(INFO) << "loading builtin core.js";
      TryCatch trycatch;
      Local<Script> script = GetCoreScript();
      HandleError(trycatch);
      script->Run();
      HandleError(trycatch);
      LOG(INFO) << "finished loading builtin core.js";
    }

    // sequentially load any other scripts
    for (auto it = scripts_.begin(); it != scripts_.end(); ++it) {
      LOG(INFO) << "loading additional script \"" << *it << "\"";
      TryCatch trycatch;
      Handle<String> source = js::ReadFile(*it);
      Handle<Script> scr = Script::New(
          source, String::New(it->c_str(), it->size()));
      HandleError(trycatch);
      scr->Run();
      HandleError(trycatch);
      LOG(INFO) << "finished loading additional script \"" << *it << "\"";
    }
  }

  // run the callback
  if (!bail && keep_going) {
    then(context);
  }
  DisposeContext();
}

std::vector<Buffer*> * State::GetBuffers(void) {
  return &buffers_;
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
