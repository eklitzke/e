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
std::vector<Handle<Object> > & EventListener::CallbackMap(
    const std::string &callback_name,
    bool use_capture) {
  if (use_capture)
    return capture_[callback_name];
  else
    return bubble_[callback_name];
}

bool EventListener::Add(const std::string& callback_name,
                        Handle<Object> callback,
                        bool use_capture) {
  std::vector<Handle<Object> > &callback_list = CallbackMap(callback_name,
                                                            use_capture);
  std::vector<Handle<Object> >::iterator it;
  for (it = callback_list.begin(); it != callback_list.end(); ++it) {
    if ((*it) == callback) {
      return false;
    }
  }
  callback_list.push_back(callback);
  return true;
}

bool EventListener::Remove(const std::string &callback_name,
                           Handle<Object>  callback,
                           bool use_capture) {
  std::vector<Handle<Object> > &callback_list = CallbackMap(callback_name,
                                                            use_capture);
  std::vector<Handle<Object> >::iterator it;
  for (it = callback_list.begin(); it != callback_list.end(); ++it) {
    if (*it == callback) {
      callback_list.erase(it);
      return true;
    }
  }
  return false;
}

bool EventListener::CallHandler(Handle<Value> h,
                                Handle<Object> this_argument,
                                size_t argc,
                                Handle<Value> argv[]) {
  HandleScope scope;
  if (h->IsObject()) {
    Handle<Object> o = Handle<Object>::Cast(h);
    if (o->IsCallable()) {
      o->CallAsFunction(this_argument, argc, argv);
      return true;
    }

    Handle<String> handle_event_string = String::NewSymbol("handleEvent");
    Handle<Value> handle_event = o->Get(handle_event_string);
    if (handle_event->IsObject()) {
      Handle<Object> handle_event_obj = Handle<Object>::Cast(handle_event);
      if (handle_event_obj->IsCallable()) {
        handle_event_obj->CallAsFunction(this_argument, argc, argv);
        return false;
      }
    }
  }
  return false;
}

void EventListener::Dispatch(const std::string& name) {
  Handle<Object> globals = GetContext()->Global();
  std::vector<Handle<Value> > arguments;
  Dispatch(name, globals, arguments);
}

void EventListener::Dispatch(const std::string& name,
                             const std::vector<Handle<Value> >& args) {
  Handle<Object> globals = GetContext()->Global();
  Dispatch(name, globals, args);
}

void EventListener::Dispatch(const std::string &name,
                             Handle<Object> this_argument,
                             const std::vector<Handle<Value> > &arguments) {
  std::vector<Handle<Object> > &captures = capture_[name];
  std::vector<Handle<Object> > &bubbles = bubble_[name];

  std::vector<Handle<Object> >::iterator it;
  std::vector<Handle<Value> >::const_iterator cit;

  // build up argc and argv
  const size_t argc = arguments.size();
  boost::scoped_array<Handle<Value> > argv(new Handle<Value>[argc]);
  int i = 0;
  for (cit = arguments.begin(); cit != arguments.end(); ++cit) {
    argv[i++] = Handle<Value>(*cit);
  }

  // call all of the capture callbacks
  for (it = captures.begin(); it != captures.end(); ++it) {
    CallHandler(*it, this_argument, argc, argv.get());
  }

  // call all of the bubble callbacks
  for (it = bubbles.begin(); it != bubbles.end(); ++it) {
    CallHandler(*it, this_argument, argc, argv.get());
  }
}

// Reads a file into a v8 string.
Handle<String> ReadFile(const std::string& name) {
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

  Handle<String> result = String::New(chars.get(), size);
  return result;
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

// Convert a JavaScript string to a std::string.  To not bother too
// much with string encodings we just use ascii.
std::string ValueToString(Local<Value> value) {
  String::Utf8Value utf8_value(value);
  return std::string(*utf8_value);
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
