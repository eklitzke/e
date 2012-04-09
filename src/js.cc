// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./js.h"

#include <boost/scoped_array.hpp>

#include <map>
#include <string>
#include <vector>

#include "./assert.h"
#include "./embeddable.h"
#include "./logging.h"
#include "./module.h"

using v8::Arguments;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Integer;
using v8::Message;
using v8::Object;
using v8::Script;
using v8::StackFrame;
using v8::StackTrace;
using v8::String;
using v8::Undefined;
using v8::Value;

namespace {
// @class: Global
// @description: The globals object

// @method: log
// @param[msg]: #string Log message
// @description: Logs a message (with the file name and line number)
Handle<Value> JSLog(const Arguments& args) {
  CHECK_ARGS(1);
  Local<Value> arg = args[0];
  String::Utf8Value msg(arg);
  std::string std_msg(*msg, msg.length());

  // Extract a stack trace
  Local<StackTrace> trace = StackTrace::CurrentStackTrace(1);
  Local<StackFrame> top = trace->GetFrame(0);
  String::Utf8Value script_name(top->GetScriptName());
  std::string std_name(*script_name, script_name.length());
  int line_no = top->GetLineNumber();
  e::LOG(e::SCRIPT, "<%s:%d> %s", std_name.c_str(), line_no, std_msg.c_str());
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
      e::LOG(e::INFO, "Assertion failed: %s", *value);
      e::Panic("JavaScript assert() failed: %s", *value);
    } else {
      e::Panic("JavaScript assert() failed (no error message was provided)");
    }
  }
  return Undefined();
}

// @method: panic
// @param[message]: #string Message to print
Handle<Value> JSPanic(const Arguments& args) {
  CHECK_ARGS(1);
  Local<Value> msg = args[0];
  String::Utf8Value value(msg);
  e::Panic(*value);
  return Undefined();
}

// @method: require
// @param[file]: #string The module to load
// @description: Loads a new JavaScript module
Handle<Value> JSRequire(const Arguments& args) {
  CHECK_ARGS(1);
  String::Utf8Value value(args[0]->ToString());
  std::string script_name(*value, value.length());
  return e::GetModule(script_name);
}
}

namespace e {
void HandleError(const TryCatch &try_catch) {
  HandleScope scope;
  if (try_catch.HasCaught()) {
    Local<Value> exc = try_catch.Exception();
    Local<Message> message = try_catch.Message();
    String::AsciiValue exception_str(exc);
    if (!message.IsEmpty()) {
      Handle<StackTrace> trace = message->GetStackTrace();
      if (!trace.IsEmpty()) {
        Local<StackFrame> top = trace->GetFrame(0);
        String::Utf8Value script_name(top->GetScriptName());
        int line_no = top->GetLineNumber();
        e::Panic("<%s:%d>: %s\n", *script_name, line_no, *exception_str);
      } else {
        int line_no = message->GetLineNumber();
        e::Panic("<unknown:%d>: %s\n", line_no, *exception_str);
      }
    } else {
      e::Panic("<unknown>: %s\n", *exception_str);
    }
    ASSERT(false);  // one of the above should have panicked
  }
}

namespace js {
// Reads a file into a v8 string.
Local<String> ReadFile(const std::string& name, bool prefix_use_strict) {
  HandleScope scope;
  FILE* file = fopen(name.c_str(), "rb");
  if (file == nullptr) {
    return String::New("");
  }

  ASSERT(fseek(file, 0, SEEK_END) == 0);
  long size = ftell(file);  // NOLINT
  ASSERT(size > 0);
  rewind(file);

  boost::scoped_array<char> chars(new char[size + 1]);
  chars[size] = '\0';
  for (long i = 0; i < size;) {  // NOLINT
    size_t bytes_read = fread(&chars[i], 1, size - i, file);
    ASSERT(bytes_read > 0);
    i += bytes_read;
  }
  ASSERT(fclose(file) == 0);
  LOG(DBG, "ReadFile() read %zd bytes from \"%s\"", size, name.c_str());

  if (prefix_use_strict) {
    std::string strict = "\"use strict\";\n";
    strict += std::string(chars.get(), size);
    Local<String> result = String::New(strict.c_str(), strict.size());
    return scope.Close(result);
  } else {
    Local<String> result = String::New(chars.get(), size);
    return scope.Close(result);
  }
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

void AddJsToGlobalNamespace(Local<ObjectTemplate> global) {
  global->Set(String::NewSymbol("assert"),
              FunctionTemplate::New(JSAssert), v8::ReadOnly);
  global->Set(String::NewSymbol("log"),
              FunctionTemplate::New(JSLog), v8::ReadOnly);
  global->Set(String::NewSymbol("panic"),
              FunctionTemplate::New(JSPanic), v8::ReadOnly);
  global->Set(String::NewSymbol("require"),
              FunctionTemplate::New(JSRequire), v8::ReadOnly);
}
}
