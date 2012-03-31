// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./embeddable.h"

#include <v8.h>

#include "./assert.h"

using v8::External;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Local;
using v8::Value;

namespace e {
namespace {
bool is_initialized = false;
Persistent<Context> context_;
}

Persistent<Context> InitializeContext(Handle<ObjectTemplate> global) {
  ASSERT(is_initialized == false);
  context_ = Context::New(nullptr, global);
  is_initialized = true;
  return context_;
}

Persistent<Context> GetContext() {
  ASSERT(is_initialized == true);
  return context_;
}

void DisposeContext() {
  ASSERT(is_initialized == true);
  context_.Dispose();
  is_initialized = false;
}
}
