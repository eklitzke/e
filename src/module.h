// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_MODULE_H_
#define SRC_MODULE_H_

#include <v8.h>
#include <string>

using v8::Handle;
using v8::Object;
using v8::Persistent;
using v8::Value;

namespace e {
typedef bool (*ModuleBuilder)(Handle<Object>);

void DeclareModule(const std::string &name, ModuleBuilder builder);
Persistent<Value> GetModule(const std::string &name);
void AddFunction(Handle<Object>, const std::string&, v8::InvocationCallback);
}
#endif  // SRC_MODULE_H_
