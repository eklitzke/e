// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./event_listener.h"

#include <boost/scoped_array.hpp>
//#include <glog/logging.h>
//#include <glog/log_severity.h>

#include <map>
#include <string>
#include <vector>

#include "./embeddable.h"
#include "./js.h"

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
std::vector<Handle<Object> >& EventListener::CallbackMap(
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

  // build up argc and argv
  const size_t argc = arguments.size();
  boost::scoped_array<Handle<Value> > argv(new Handle<Value>[argc]);
  int i = 0;
  for (auto cit = arguments.begin(); cit != arguments.end(); ++cit) {
    argv[i++] = Handle<Value>(*cit);
  }

  // call all of the capture callbacks
  for (auto it = captures.begin(); it != captures.end(); ++it) {
    CallHandler(*it, this_argument, argc, argv.get());
  }

  // call all of the bubble callbacks
  for (auto it = bubbles.begin(); it != bubbles.end(); ++it) {
    CallHandler(*it, this_argument, argc, argv.get());
  }
}
}
