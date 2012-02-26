// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_EMBEDDABLE_H_
#define SRC_EMBEDDABLE_H_

#include <v8.h>
#include <map>
#include <string>
#include <vector>

namespace e {

using v8::Context;
using v8::FunctionTemplate;
using v8::InvocationCallback;
using v8::Handle;
using v8::Local;
using v8::Persistent;
using v8::Value;

class Embeddable {
 public:
  virtual ~Embeddable() { context_.Dispose(); }
  Local<FunctionTemplate> ToCallable(InvocationCallback);
  Local<Value> ToExternal();
  template <typename T> static T *FromExternal(Handle<Value>);

 protected:
  Persistent<Context> context_;
};
}

#endif  // SRC_EMBEDDABLE_H_
