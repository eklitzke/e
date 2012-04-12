// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_JS_H_
#define SRC_JS_H_

#include <v8.h>
#include <map>
#include <string>
#include <vector>

using v8::AccessorInfo;
using v8::Arguments;
using v8::Context;
using v8::External;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Integer;
using v8::InvocationCallback;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::Persistent;
using v8::String;
using v8::TryCatch;
using v8::Value;

#define CHECK_ARGS(num) if (args.Length() < num) {  \
    return v8::Undefined();                         \
  }                                                 \
  HandleScope scope

namespace e {
namespace js {

void AddTemplateFunction(Handle<ObjectTemplate>,
                         const std::string &,
                         v8::InvocationCallback);
void AddTemplateAccessor(Handle<ObjectTemplate>,
                         const std::string &,
                         v8::AccessorGetter,
                         v8::AccessorSetter);

typedef Handle<Value>(*JSCallback)(const Arguments&);
typedef Handle<Value>(*JSAccessor)(Local<String>, const AccessorInfo&);

// Reads a file into a v8 string.
Local<v8::String> ReadFile(const std::string& name,
                            bool prefix_use_strict = false);
std::string ValueToString(Local<Value>);
}

// Handle caught errors
void HandleError(const TryCatch&);

// mutate the globals dictionary
void AddJsToGlobalNamespace(Local<ObjectTemplate> global);

// Get the errno saved by SaveErrno().
Local<Integer> GetErrno();

// Save the value of errno. In general this is needed because when interacting
// with V8, there's no guarantee that V8 won't do anything that could set
// errno. Therefore, when passing the errno value between different parts of the
// program, it may be necessary to set and restore the errno.
void SaveErrno();
}

#endif  // SRC_JS_H_
