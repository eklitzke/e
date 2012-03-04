// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <v8.h>
#include <glog/logging.h>

#include <cassert>
#include <csignal>
#include <string>

#include "./js.h"
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

Handle<Value> JSGetPid(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Integer::New(static_cast<int>(getpid())));
}

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

State::State(const std::string &script_name)
    :active_buffer_(new Buffer("*temp*")), script_name_(script_name) {
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

  Handle<ObjectTemplate> window_templ = ObjectTemplate::New();
  window_templ->SetInternalFieldCount(1);
  window_templ->Set(String::NewSymbol("addEventListener"),
                    FunctionTemplate::New(AddEventListener), v8::ReadOnly);
  window_templ->Set(String::NewSymbol("stopLoop"),
                    FunctionTemplate::New(JSStopLoop), v8::ReadOnly);

  Handle<ObjectTemplate> sys_templ = ObjectTemplate::New();
  sys_templ->Set(String::NewSymbol("getpid"),
                    FunctionTemplate::New(JSGetPid), v8::ReadOnly);
  sys_templ->Set(String::NewSymbol("kill"),
                    FunctionTemplate::New(JSKill), v8::ReadOnly);
  sys_templ->Set(String::NewSymbol("SIGTSTP"),
                 Integer::New(SIGTSTP), v8::ReadOnly);

  // add in all of the movement callbacks
  std::map<std::string, e::js::JSCallback> callbacks = e::js::GetCallbacks();
  std::map<std::string, e::js::JSCallback>::iterator cit;
  Handle<ObjectTemplate> curses = ObjectTemplate::New();
  for (cit = callbacks.begin(); cit != callbacks.end(); ++cit) {
    curses->Set(String::NewSymbol(cit->first.c_str()),
                FunctionTemplate::New(cit->second), v8::ReadOnly);
  }

  context_ = Context::New(nullptr, global);
  Context::Scope context_scope(context_);

  Local<Object> window = window_templ->NewInstance();
  window->SetInternalField(0, External::New(this));
  window->Set(String::NewSymbol("buffer"), active_buffer_->ToScript(), v8::ReadOnly);
  context_->Global()->Set(String::NewSymbol("window"), window, v8::ReadOnly);
  context_->Global()->Set(String::NewSymbol("curses"), curses->NewInstance(), v8::ReadOnly);
  context_->Global()->Set(String::NewSymbol("sys"), sys_templ->NewInstance(), v8::ReadOnly);

  bool bail = false;
  if (run) {
    // compile the JS source code, and run it once


    TryCatch trycatch;
    Handle<String> source = js::ReadFile(script_name_);
    Handle<Script> scr = Script::New(
        source, String::New(script_name_.c_str(), script_name_.size()));
    if (scr.IsEmpty()) {
      Handle<Value> exception = trycatch.Exception();
      String::AsciiValue exception_str(exception);
      fprintf(stderr, "Exception: %s\n", *exception_str);
      bail = true;
    } else {
      Handle<Value> v = scr->Run();
      if (v.IsEmpty()) {
        Handle<Value> exception = trycatch.Exception();
        String::AsciiValue exception_str(exception);
        fprintf(stderr, "Exception: %s\n", *exception_str);
        bail = true;
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
