// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef JS_H_
#define JS_H_

#include <v8.h>
#include <map>
#include <string>
#include <vector>

namespace e {
namespace js {
class EventListener {
 public:
  bool add(const std::string&, v8::Handle<v8::Object> *, bool);
  bool remove(const std::string&, v8::Handle<v8::Object> *, bool);
  void dispatch(const std::string&, v8::Handle<v8::Value> *,
                const std::vector<v8::Handle<v8::Value>*> &);
  void install_in_prototype(v8::Object *);
 private:
  std::map<std::string, std::vector<v8::Handle<v8::Object>*> > capture_;
  std::map<std::string, std::vector<v8::Handle<v8::Object>*> > bubble_;

  std::vector<v8::Handle<v8::Object>*> & callback_map(const std::string &,
                                                      bool);
};

// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile(const std::string& name);
v8::Handle<v8::Value> LogCallback(const v8::Arguments& args);
}
}

#endif  // JS_H_
