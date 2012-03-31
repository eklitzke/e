// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./module.h"

#include <v8.h>
#include <map>
#include <string>

#include "./assert.h"

using v8::Function;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::String;

namespace {
std::map<std::string, Persistent<Object> > modules_;
}

namespace e {

void DeclareModule(const std::string &name, ModuleBuilder builder) {
  auto it = modules_.find(name);
  ASSERT(it == modules_.end());

  HandleScope scope;
  Local<Object> module_obj = Object::New();
  ASSERT(builder(module_obj));
  modules_[name] = Persistent<Object>::New(module_obj);
}

Persistent<Object> GetModule(const std::string &name) {
  auto it = modules_.find(name);
  ASSERT(it != modules_.end());  // else look up module
  return it->second;
}

void AddFunction(Handle<Object> obj, const std::string &name,
                 v8::InvocationCallback func) {
  HandleScope scope;
  Persistent<Function> pfunc = Persistent<Function>::New(
      FunctionTemplate::New(func)->GetFunction());
  obj->Set(String::NewSymbol(name.c_str()), pfunc, v8::ReadOnly);
}
}
