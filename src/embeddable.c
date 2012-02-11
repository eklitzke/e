// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./embeddable.h"

Local<FunctionTemplate>
Embeddable::ToCallable(InvocationCallback func) {
  HandleScope scope;
  Local<Value> ext = Function
  Local<FunctionTemplate> funcTemplate = FunctionTemplate::New(func, classPtrToExternal());
  return scope.Close(funcTemplate);
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
