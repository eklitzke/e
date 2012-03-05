// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <curses.h> // naughty, for OK and ERR
#include <v8.h>
#include <glog/logging.h>

#include <cassert>
#include <string>

#include "./bundled_core.h"
#include "./js.h"
#include "./js_curses.h"
#include "./js_curses_window.h"
#include "./js_errno.h"
#include "./js_signal.h"
#include "./js_sys.h"
#include "./state.h"

namespace e {

using v8::Arguments;
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

Handle<Value> JSStopLoop(const Arguments& args) {
  keep_going = false;
  return Undefined();
}

Handle<Value>
AddEventListener(const Arguments& args) {
  if (args.Length() < 2) {
    return Undefined();
  }

  Local<Object> self = args.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  State* state = reinterpret_cast<State*>(wrap->Value());

  HandleScope scope;

  // XXX: just cast the first argument to a string?
  Local<String> event_name = args[0]->ToString();

  Handle<Value> callback = args[1];
  if (!callback->IsObject()) {
    return Undefined();
  }
  state->callback_o = Persistent<Object>::New(callback->ToObject());
  google::FlushLogFiles(google::INFO);

  bool use_capture = true;
  if (args.Length() >= 3) {
    use_capture = args[2]->BooleanValue();
  }

  state->GetListener()->Add(
      js::ValueToString(event_name), state->callback_o, use_capture);

  return Undefined();
}
}

State::State(bool load_core, const std::vector<std::string> &scripts)
    :load_core_(load_core), scripts_(scripts),
     active_buffer_(new Buffer("*temp*")) {
  buffers_.push_back(active_buffer_);
}

State::~State() {
  for (auto it = buffers_.begin(); it != buffers_.end(); ++it) {
    delete *it;
  }
}

void State::LoadScript(bool run, boost::function<void(Persistent<Context>)> then) {

  HandleScope scope;
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  global->Set(String::NewSymbol("log"),
              FunctionTemplate::New(js::LogCallback), v8::ReadOnly);

  Handle<ObjectTemplate> world_templ = ObjectTemplate::New();
  world_templ->SetInternalFieldCount(1);
  world_templ->Set(String::NewSymbol("addEventListener"),
                   FunctionTemplate::New(AddEventListener), v8::ReadOnly);
  world_templ->Set(String::NewSymbol("stopLoop"),
                   FunctionTemplate::New(JSStopLoop), v8::ReadOnly);

  // added in the curses object
  Handle<ObjectTemplate> curses = ObjectTemplate::New();
  NEW_INTEGER(curses, OK);
  NEW_INTEGER(curses, ERR);
  std::map<std::string, e::js::JSCallback> callbacks = GetCursesCallbacks();
  for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
    js::AddTemplateFunction(curses, it->first, it->second);
  }
  std::map<std::string, js::JSAccessor> accessors = GetCursesAccessors();
  for (auto it = accessors.begin(); it != accessors.end(); ++it) {
    curses->SetAccessor(String::NewSymbol(it->first.c_str()), it->second);
  }

  context_ = Context::New(nullptr, global);
  Context::Scope context_scope(context_);

  Local<Object> world = world_templ->NewInstance();
  world->SetInternalField(0, External::New(this));
  world->Set(String::NewSymbol("buffer"), active_buffer_->ToScript(), v8::ReadOnly);
  context_->Global()->Set(String::NewSymbol("world"), world, v8::ReadOnly);

  Local<Object> curses_obj = curses->NewInstance();

  context_->Global()->Set(String::NewSymbol("curses"), curses_obj, v8::ReadOnly);

  context_->Global()->Set(String::NewSymbol("errno"), GetErrnoTemplate()->NewInstance(), v8::ReadOnly);
  context_->Global()->Set(String::NewSymbol("signal"), GetSignalTemplate()->NewInstance(), v8::ReadOnly);
  context_->Global()->Set(String::NewSymbol("sys"), GetSysTemplate()->NewInstance(), v8::ReadOnly);

  bool bail = false;
  if (run) {
    // compile the JS source code, and run it
    if (load_core_) {
      // load the core file; this should be known to be good and not throw
      // exceptions
      Local<Script> core_scr = GetCoreScript();
      core_scr->Run();
    }
    for (auto it = scripts_.begin(); it != scripts_.end(); ++it) {
      TryCatch trycatch;
      Handle<String> source = js::ReadFile(*it);
      Handle<Script> scr = Script::New(
          source, String::New(it->c_str(), it->size()));
      if (scr.IsEmpty()) {
        Handle<Value> exception = trycatch.Exception();
        String::AsciiValue exception_str(exception);
        fprintf(stderr, "Exception: %s\n", *exception_str);
        bail = true;
        break;
      }
      Handle<Value> v = scr->Run();
      if (v.IsEmpty()) {
        Handle<Value> exception = trycatch.Exception();
        String::AsciiValue exception_str(exception);
        fprintf(stderr, "Exception: %s\n", *exception_str);
        bail = true;
        break;
      }
    }
  }
  if (!bail)
    then(context_);
}

Buffer *
State::GetActiveBuffer(void) {
  return active_buffer_;
}

std::vector<Buffer*> *
State::GetBuffers(void) {
  return &buffers_;
}

bool
State::HandleKey(KeyCode *k) {
  HandleScope scope;

  std::vector<Handle<Value> > args;
  args.push_back(k->ToScript());
  listener_.Dispatch("keypress", context_->Global(), args);

  return keep_going;
}
}
