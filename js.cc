// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <map>
#include <string>
#include <vector>

#include "./js.h"
#include "./log.h"

using namespace v8;

namespace e {
namespace js {

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

// Reads a file into a v8 string.
Handle<String> ReadFile(const std::string& name) {
  FILE* file = fopen(name.c_str(), "rb");
  if (file == NULL) {
    return Handle<String>();
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);

  Handle<String> result = String::New(chars, size);
  delete[] chars;
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
