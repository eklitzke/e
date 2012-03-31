// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_EVENT_LISTENER_H_
#define SRC_EVENT_LISTENER_H_

#include <v8.h>
#include <map>
#include <string>
#include <vector>

using v8::Handle;
using v8::Object;
using v8::Value;

namespace e {
class EventListener {
 public:
  bool Add(const std::string&, Handle<Object>, bool);
  bool Remove(const std::string&, Handle<Object>, bool);
  void Dispatch(const std::string& name);
  void Dispatch(const std::string& name,
                const std::vector<Handle<Value> >& args);
  void Dispatch(const std::string& name, Handle<Object> this_object,
                const std::vector<Handle<Value> >& args);
 private:
  std::map<std::string, std::vector<Handle<Object> > > capture_;
  std::map<std::string, std::vector<Handle<Object> > > bubble_;

  bool CallHandler(Handle<Value> h, Handle<Object>, size_t, Handle<Value>[]);
  std::vector<Handle<Object> >& CallbackMap(const std::string &, bool);
};
}

#endif  // SRC_EVENT_LISTENER_H_
