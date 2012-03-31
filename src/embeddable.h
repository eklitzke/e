// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_EMBEDDABLE_H_
#define SRC_EMBEDDABLE_H_

#include <v8.h>
#include <map>
#include <string>
#include <vector>

using v8::Arguments;
using v8::Context;
using v8::External;
using v8::FunctionTemplate;
using v8::InvocationCallback;
using v8::Handle;
using v8::HandleScope;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::Persistent;
using v8::String;
using v8::Value;

namespace e {
template<typename T> T* Unwrap(Handle<Object> holder, int field = 0) {
  Local<External> wrap = Local<External>::Cast(holder->GetInternalField(field));
}

template<typename T> T* UnwrapObj(Handle<Object> obj, int field = 0) {
  HandleScope scope;
  Local<External> wrap = Local<External>::Cast(obj->GetInternalField(field));
  return static_cast<T*>(wrap->Value());
}


// Unwrap an External from an Arguments reference
template<typename T, typename A> T* Unwrap(const A &args, int field = 0) {
  HandleScope scope;
  Local<Object> holder = args.Holder();
  return UnwrapObj<T>(holder, field);
}

#define GET_SELF2(a, tp) Local<Object> s_ = a.Holder();\
  v8::Local<v8::External> wrap = \
      v8::Local<v8::External>::Cast(s_->GetInternalField(0)); \
  tp* self = static_cast<tp*>(wrap->Value())

#define GET_SELF(tp) GET_SELF2(args, tp)
#define ACCESSOR_GET_SELF(tp) GET_SELF2(info, tp)

Persistent<Context> InitializeContext(Handle<ObjectTemplate> global);
Persistent<Context> GetContext();  // FIXME(eklitzke): use GetCurrent() instead?
void DisposeContext();
}

#endif  // SRC_EMBEDDABLE_H_
