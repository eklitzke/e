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
        Panic("<%s:%d>: %s\n", *script_name, line_no, *exception_str);
      } else {
        int line_no = message->GetLineNumber();
        Panic("<unknown:%d>: %s\n", line_no, *exception_str);
      }
    } else {
      Panic("<unknown>: %s\n", *exception_str);
    }
    assert(false);  // one of the above should have panicked
  }
}

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
// @param[flush]: #bool whether to flush after logging (optional, default false)
// @description: Logs a message (with the file name and line number)
Handle<Value> JSLog(const Arguments& args) {
  CHECK_ARGS(1);
  Local<Value> arg = args[0];
  bool flush = false;
  if (args.Length() >= 2) {
    flush = args[1]->BooleanValue();
  }
  String::Utf8Value msg(arg);
  std::string std_msg(*msg, msg.length());

  // Extract a stack trace
  Local<StackTrace> trace = StackTrace::CurrentStackTrace(1);
  Local<StackFrame> top = trace->GetFrame(0);
  String::Utf8Value script_name(top->GetScriptName());
  std::string std_name(*script_name, script_name.length());
  int line_no = top->GetLineNumber();
  LOG(INFO) << "<" << std_name << ":" << line_no << "> " << std_msg;
  if (flush) {
    google::FlushLogFiles(google::INFO);
  }
  return Undefined();
}

// @method: flushLogs
// @description: Flush all logs.
Handle<Value> JSFlushLogs(const Arguments& args) {
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
      Panic("JavaScript assert() failed: %s", *value);
    } else {
      Panic("JavaScript assert() failed (no error message was provided)");
    }
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
  return GetModule(script_name);
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
