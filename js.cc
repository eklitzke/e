// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./js.h"

#include <boost/scoped_array.hpp>

#include <map>
#include <string>
#include <vector>

#include "./log.h"

namespace e {
namespace js {

using v8::Arguments;
using v8::Handle;
using v8::HandleScope;
using v8::Object;
using v8::String;
using v8::Undefined;
using v8::Value;

std::vector<Handle<Object> *> &
EventListener::callback_map(const std::string &callback_name,
                            bool use_capture) {
  if (use_capture)
    return capture_[callback_name];
  else
    return bubble_[callback_name];
}

bool
EventListener::add(const std::string& callback_name,
                   Handle<Object> *callback,
                   bool use_capture) {
  std::vector<Handle<Object>*> &callback_list = callback_map(callback_name,
                                                              use_capture);
  std::vector<Handle<Object>*>::iterator it;
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
                      Handle<Object> *callback,
                      bool use_capture) {
  std::vector<Handle<Object>*> &callback_list = callback_map(callback_name,
                                                             use_capture);
  std::vector<Handle<Object>*>::iterator it;
  for (it = callback_list.begin(); it != callback_list.end(); ++it) {
    if (*it == callback) {
      callback_list.erase(it);
      return true;
    }
  }
  return false;
}

void
EventListener::dispatch(const std::string &name, Handle<Value> *this_argument,
                        const std::vector<Handle<Value>*> &arguments) {
  std::vector<Handle<Object>*> &captures = capture_[name];
  std::vector<Handle<Object>*> &bubbles = bubble_[name];

  std::vector<Handle<Object>*>::iterator it;
  std::vector<Handle<Value>*>::const_iterator cit;

  const size_t argc = arguments.size();
  boost::scoped_array<Handle<Value> > argv(new Handle<Value>[argc]);
  int i = 0;
  for (cit = arguments.begin();
       cit != arguments.end(); ++cit ) {
    argv[i++] = Handle<Value>(**cit);
  }

  for (it = capture_.begin(); it != capture_.end(); ++it) {
    (*it)->Call(this_argument, argc, argv.get());
  }
  for (it = bubble_.begin(); it != bubble_.end(); ++it) {
    (*it)->Call(this_argument, argc, argv.get());
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
  e::log::log_string(*value);
  return Undefined();
}
}
}
