// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./js.h"

#include <boost/scoped_array.hpp>
#include <glog/logging.h>
#include <glog/log_severity.h>

#include <map>
#include <string>
#include <vector>

#include "./assert.h"
#include "./embeddable.h"

using v8::Arguments;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Integer;
using v8::Object;
using v8::Script;
using v8::String;
using v8::Undefined;
using v8::Value;

namespace e {
namespace js {
// Reads a file into a v8 string.
Handle<String> ReadFile(const std::string& name, bool prefix_use_strict) {
  HandleScope scope;
  FILE* file = fopen(name.c_str(), "rb");
  if (file == nullptr) {
    return Handle<String>();
  }

  ASSERT(fseek(file, 0, SEEK_END) == 0);
  int size = ftell(file);
  ASSERT(size > 0);
  rewind(file);

  boost::scoped_array<char> chars(new char[size + 1]);
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    ssize_t read = fread(&chars[i], 1, size - i, file);
    ASSERT(read >= 0);
    i += read;
  }
  ASSERT(fclose(file) == 0);

  if (prefix_use_strict) {
    std::string strict = "\"use strict\";\n";
    strict += std::string(chars.get(), size);
    Handle<String> result = String::New(strict.c_str(), strict.size());
    return scope.Close(result);
  } else {
    Handle<String> result = String::New(chars.get(), size);
    return scope.Close(result);
  }
}

// @class: Global
// @description: The globals object

// @method: log
// @param[msg]: #string Log message
// @description: Logs a message
Handle<Value> JSLog(const Arguments& args) {
  CHECK_ARGS(1);
  Local<Value> arg = args[0];
  String::Utf8Value value(arg);
  LOG(INFO) << (*value);
  google::FlushLogFiles(google::INFO);
  return Undefined();
}

// @method: assert
// @param[condition]: #bool Condition to check
// @param[msg]: #string Log message (optional)
// @description: Asserts truth
Handle<Value> JSAssert(const Arguments& args) {
  CHECK_ARGS(1);
  Local<Value> cond = args[0];
  if (cond->ToBoolean()->Value() == false) {
    if (args.Length() >= 2) {
      Local<Value> msg = args[1];
      String::Utf8Value value(msg);
      LOG(INFO) << "Assertion failed: " << (*value);
      google::FlushLogFiles(google::INFO);
    }
    exit(EXIT_FAILURE);
  }
  return Undefined();
}

// @method: require
// @param[file]: #string The module to load
// @description: Loads a new JavaScript module
Handle<Value> JSRequire(const Arguments& args) {
  CHECK_ARGS(1);
  String::Utf8Value value(args[0]->ToString());

  std::string script_name(*value, value.length());
  Handle<String> source = ReadFile(script_name);
  Handle<Script> scr = Script::New(
      source, String::New(script_name.c_str(), script_name.size()));
  return scr->Run();
}

// Convert a JavaScript string to a std::string (UTF-8 encoded).
std::string ValueToString(Local<Value> value) {
  String::Utf8Value utf8_value(value);
  return std::string(*utf8_value, utf8_value.length());
}

void AddTemplateFunction(Handle<ObjectTemplate> templ, const std::string &name,
                         v8::InvocationCallback callback) {
  templ->Set(String::NewSymbol(name.c_str(), name.size()),
             FunctionTemplate::New(callback),
             v8::ReadOnly);
}
void AddTemplateAccessor(Handle<ObjectTemplate> templ, const std::string &name,
                         v8::AccessorGetter getter, v8::AccessorSetter setter) {
  templ->SetAccessor(String::NewSymbol(name.c_str(), name.size()), getter,
                     setter);
}
}
}
