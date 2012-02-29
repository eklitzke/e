// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <v8.h>
#include <glog/logging.h>

#include <cassert>
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

State::State(const std::string &script_name)
    :active_buffer_(new Buffer("*temp*")), script_name_(script_name) {
}

void State::RunScript(boost::function<void()> then) {
  HandleScope handle_scope;
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  global->Set(String::NewSymbol("log"),
              FunctionTemplate::New(js::LogCallback), v8::ReadOnly);

  Handle<ObjectTemplate> window_templ = ObjectTemplate::New();
  window_templ->SetInternalFieldCount(1);
  window_templ->Set(String::NewSymbol("addEventListener"),
                    FunctionTemplate::New(AddEventListener), v8::ReadOnly);
  window_templ->Set(String::NewSymbol("stopLoop"),
                    FunctionTemplate::New(JSStopLoop), v8::ReadOnly);

  // add in all of the movement callbacks
  std::map<std::string, e::js::JSCallback> callbacks = e::js::GetCallbacks();
  std::map<std::string, e::js::JSCallback>::iterator cit;
  for (cit = callbacks.begin(); cit != callbacks.end(); ++cit) {
    DLOG(INFO) << "added JS function: window." << cit->first;
    window_templ->Set(String::NewSymbol(cit->first.c_str()),
                      FunctionTemplate::New(cit->second), v8::ReadOnly);
  }

  context_ = Context::New(nullptr, global);
  Context::Scope context_scope(context_);

  Local<Object> window = window_templ->NewInstance();
  window->SetInternalField(0, External::New(this));
  context_->Global()->Set(String::New("window"), window, v8::ReadOnly);

  // compile the JS source code, and run it once
  Handle<String> source = js::ReadFile(script_name_);
  Handle<Script> scr = Script::Compile(source);

  scr->Run();
  then();
}

Buffer *
State::GetActiveBuffer(void) {
  return active_buffer_;
}

std::vector<Buffer *> *
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
