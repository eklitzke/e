// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// Core functionality for implementing CommonJS modules.

#ifndef SRC_MODULE_H_
#define SRC_MODULE_H_

#include <v8.h>
#include <string>

using v8::Handle;
using v8::Object;
using v8::Persistent;
using v8::Value;

namespace e {
// This is the typedef for a valid module builder. The module builder will
// accept a handler to an Object, and then return true if it was able to
// successfully initialize the object (and return false otherwise).
typedef bool (*ModuleBuilder)(Handle<Object>);

// Declare a builtin module; unless you *really* know what you're doing, this
// function should only be called from InitializeBuiltinModules() in
// module_decl.cc.
void DeclareBuiltinModule(const std::string &name, ModuleBuilder builder);

// Load a module; this could be a builtin module or a module that should be
// loaded from the filesystem. After the module has been loaded it will be added
// to an internal module cache, and future accesses for that module will return
// the module from the cache.
Persistent<Value> GetModule(const std::string &name);

// Add an accessor to a module dictionary (useful for builtin modules)
void AddAccessor(Handle<Object>, const std::string &,
                 v8::AccessorGetter, v8::AccessorSetter = nullptr);

// Add a function to a module dictionary (useful for builtin modules)
void AddFunction(Handle<Object>, const std::string&, v8::InvocationCallback);

// Add a static integer to a module dictionary (useful for builtin modules)
void AddInteger(Handle<Object>, const std::string&, int);
}
#endif  // SRC_MODULE_H_
