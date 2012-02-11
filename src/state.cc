// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <v8.h>

#include <cassert>
#include <string>

#include "./js.h"
#include "./state.h"

namespace e {

using v8::Arguments;
using v8::Context;
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

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

State::State(const std::string &script_name)
    :active_buffer_(new Buffer("*temp*")) {

  HandleScope handle_scope;

  Handle<FunctionTemplate> window_obj = FunctionTemplate::New();
  //Local<Template> proto_t = window_obj->PrototypeTemplate();
  //proto_t->Set("addEventListener", v8::FunctionTemplate::New(addEventListener));


  Handle<ObjectTemplate> global = ObjectTemplate::New();
  global->Set(String::New("log"), FunctionTemplate::New(js::LogCallback));
  global->Set(String::New("window"), window_obj);

  // compile the JS source code, and run it once
  Handle<String> source = js::ReadFile(script_name);
  Handle<Script> scr = Script::Compile(source);
  scr->Run();

  // get the onKeyPress function
  Handle<String> keypress_cb_name = String::New("onKeyPress");
  Handle<Value> onkeypress_val = context_->Global()->Get(keypress_cb_name);
  if (!onkeypress_val->IsFunction()) {
    assert(false);
  }

  // cast it to a function
  Handle<Function> onkeypress_fun = Handle<Function>::Cast(onkeypress_val);

  // that to remain after this call returns
  onkeypress_ = Persistent<Function>::New(onkeypress_fun);
}

Handle<Value>
State::addEventListener(const Arguments& args) {
  if (args.Length() < 2) {
    return Undefined();
  }
  HandleScope scope;

  // XXX: just cast the first argument to a string?
  Handle<String> event_name = args[0]->ToString();

  Handle<Value> callback = args[1];
  if (!callback->IsObject()) {
    return Undefined();
  }
  Handle<Object> callback_o = callback->ToObject();

  bool use_capture = true;
  if (args.Length() >= 3) {
    use_capture = args[2]->BooleanValue();
  }

  //listener_.add(js::ValueToString(event_name), callback_o, use_capture);
  return Undefined();
}

Buffer *
State::get_active_buffer(void) {
  return active_buffer_;
}

std::vector<Buffer *> *
State::get_buffers(void) {
  return &buffers_;
}

bool
State::handle_key(const KeyCode &k) {
  if (k.is_ascii() && k.get_char() == 'q') {
    return false;
  }

  HandleScope scope;

  const int argc = 1;
  Handle<Value> argv[argc] = { Integer::New(k.get_code()) };
  onkeypress_->Call(context_->Global(), argc, argv);

  return true;
}
}
