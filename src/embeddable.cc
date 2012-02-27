// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./embeddable.h"

#include <v8.h>

namespace e {

using v8::External;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Local;
using v8::Value;

Local<FunctionTemplate>
Embeddable::ToCallable(InvocationCallback func) {
  HandleScope scope;
  Local<FunctionTemplate> func_template = FunctionTemplate::New(
      func, ToExternal());
  return scope.Close(func_template);
}

Local<Value>
Embeddable::ToExternal() {
  HandleScope scope;
  return scope.Close(External::Wrap(reinterpret_cast<void *>(this)));
}

template <typename T>
T* Embeddable::FromExternal(Handle<Value> data) {
  if (!data.IsEmpty() && data->IsExternal())
    return static_cast<T *>(External::Unwrap(data));
  return NULL;
}
}
