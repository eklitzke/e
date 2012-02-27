// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./js.h"

#include <boost/scoped_array.hpp>
#include <glog/logging.h>
#include <glog/log_severity.h>

#include <map>
#include <string>
#include <vector>

#ifdef USE_CURSES
#include <curses.h>
#endif

namespace e {
namespace js {

using v8::Arguments;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Object;
using v8::String;
using v8::Undefined;
using v8::Value;

std::vector<Handle<Object> > &
EventListener::callback_map(const std::string &callback_name,
                            bool use_capture) {
  if (use_capture)
    return capture_[callback_name];
  else
    return bubble_[callback_name];
}

bool
EventListener::add(const std::string& callback_name,
                   Handle<Object> callback,
                   bool use_capture) {
  std::vector<Handle<Object> > &callback_list = callback_map(callback_name,
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

bool
EventListener::remove(const std::string &callback_name,
                      Handle<Object>  callback,
                      bool use_capture) {
  std::vector<Handle<Object> > &callback_list = callback_map(callback_name,
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

bool
EventListener::call_handler(Handle<Value> h,
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

void
EventListener::dispatch(const std::string &name, Handle<Object> this_argument,
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
    call_handler(*it, this_argument, argc, argv.get());
  }

  // call all of the bubble callbacks
  for (it = bubbles.begin(); it != bubbles.end(); ++it) {
    call_handler(*it, this_argument, argc, argv.get());
  }
}

// Reads a file into a v8 string.
Handle<String> ReadFile(const std::string& name) {
  FILE* file = fopen(name.c_str(), "rb");
  if (file == NULL) {
    return Handle<String>();
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  boost::scoped_array<char> chars(new char[size + 1]);
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);

  Handle<String> result = String::New(chars.get(), size);
  return result;
}

Handle<Value> LogCallback(const Arguments& args) {
  if (args.Length() < 1) {
    return Undefined();
  }
  HandleScope scope;
  Handle<Value> arg = args[0];
  String::Utf8Value value(arg);
  LOG(INFO) << (*value);
  google::FlushLogFiles(google::INFO);
  return Undefined();
}

//#ifdef USE_CURSES
Handle<Value> CursesAddstr(const Arguments& args) {
  if (args.Length() < 1) {
    return Undefined();
  }
  HandleScope scope;
  Handle<Value> arg = args[0];
  String::Utf8Value value(arg);
  addnstr(*value, value.length());
  refresh(); // FIXME(eklitzke): is this necessary?
  return Undefined();
}
//#endif

// Convert a JavaScript string to a std::string.  To not bother too
// much with string encodings we just use ascii.
std::string ValueToString(Local<Value> value) {
  String::Utf8Value utf8_value(value);
  return std::string(*utf8_value);
}
}
}
