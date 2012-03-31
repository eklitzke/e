// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./module.h"

#include <v8.h>
#include <map>
#include <string>

#include "./assert.h"

using v8::Function;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Integer;
using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Undefined;

namespace {
std::map<std::string, e::ModuleBuilder> builders_;
std::map<std::string, Persistent<Object> > modules_;
}

namespace e {
void DeclareModule(const std::string &name, ModuleBuilder builder) {
  auto it = builders_.find(name);
  ASSERT(it == builders_.end());
  builders_[name] = builder;
}

Persistent<Value> GetModule(const std::string &name) {
  // first check the module cache
  auto m = modules_.find(name);
  if (m != modules_.end()) {
    return m->second;
  }

  // now check the builders dict
  HandleScope scope;
  auto b = builders_.find(name);
  if (b != builders_.end()) {
    // we have a builder, build the module and cache it
    Local<Object> module_obj = Object::New();
    ASSERT(b->second(module_obj));
    Persistent<Object> p = Persistent<Object>::New(module_obj);
    modules_[name] = p;
    return p;
  }

  // next check the filesystem
  return Persistent<Value>::New(Undefined());
}

void AddAccessor(Handle<Object> obj, const std::string &name,
                 v8::AccessorGetter getter, v8::AccessorSetter setter) {
  HandleScope scope;
  obj->SetAccessor(
      String::NewSymbol(name.c_str(), name.size()), getter, setter);
}

void AddFunction(Handle<Object> obj, const std::string &name,
                 v8::InvocationCallback func) {
  HandleScope scope;
  Persistent<Function> pfunc = Persistent<Function>::New(
      FunctionTemplate::New(func)->GetFunction());
  obj->Set(String::NewSymbol(name.c_str()), pfunc, v8::ReadOnly);
}

void AddInteger(Handle<Object> obj, const std::string &name, int value) {
  obj->Set(String::NewSymbol(name.c_str()), Integer::New(value), v8::ReadOnly);
}
}
