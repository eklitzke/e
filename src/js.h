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
Handle<Value> JSAssert(const Arguments& args);
Handle<Value> JSFlushLogs(const Arguments& args);
Handle<Value> JSLog(const Arguments& args);
Handle<Value> JSPanic(const Arguments& args);
Handle<Value> JSRequire(const Arguments& args);
std::string ValueToString(Local<Value>);
}

// Handle caught errors
void HandleError(const TryCatch&);
}

#endif  // SRC_JS_H_
