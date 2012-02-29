// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//

#include "./line.h"

#include <v8.h>

#include <string>
#include <vector>

#include "./embeddable.h"

using v8::Arguments;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Undefined;
using v8::Value;

#define RETURN_SELF return scope.Close(String::New(self->value.c_str(), self->value.length()))

namespace e {

Line::Line(const std::string &line)
    :value(line) {
}

void Line::Replace(const std::string &new_line) {
  value = new_line;
  for (auto it = callbacks_.begin(); it != callbacks_.end(); ++it) {
    (*it)(new_line);
  }
}

const std::string& Line::ToString() const {
  return value;
}

void Line::OnChange(StringCallback cb) {
  callbacks_.push_back(cb);
}

namespace {
Handle<Value> JSErase(const Arguments& args) {
  GET_SELF(Line);
  if (args.Length() < 2) {
    return Undefined();
  }

  HandleScope scope;
  Handle<Value> arg0 = args[0];
  Handle<Value> arg1 = args[1];
  uint32_t offset = arg0->Uint32Value();
  uint32_t amt = arg1->Uint32Value();
  self->value.erase(static_cast<size_t>(offset), static_cast<size_t>(amt));
  RETURN_SELF;
}

Handle<Value> JSInsert(const Arguments& args) {
  GET_SELF(Line);
  if (args.Length() < 2) {
    return Undefined();
  }

  HandleScope scope;
  Handle<Value> arg0 = args[0];
  Handle<Value> arg1 = args[1];
  uint32_t position = arg0->Uint32Value();
  String::Utf8Value chars(arg1);

  self->value.insert(static_cast<size_t>(position), *chars);
  RETURN_SELF;
}

Handle<Value> JSValue(const Arguments& args) {
  GET_SELF(Line);
  HandleScope scope;
  RETURN_SELF;
}

Persistent<ObjectTemplate> line_template;

// Create a raw template to assign to line_template
Handle<ObjectTemplate> MakeLineTemplate() {
  HandleScope handle_scope;
  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->SetInternalFieldCount(1);
  result->Set(String::NewSymbol("erase"), FunctionTemplate::New(JSErase),
    v8::ReadOnly);
  result->Set(String::NewSymbol("insert"), FunctionTemplate::New(JSInsert),
    v8::ReadOnly);
  result->Set(String::NewSymbol("value"), FunctionTemplate::New(JSValue),
    v8::ReadOnly);
  return handle_scope.Close(result);
}
}

Handle<Value>
Line::ToScript() {
  HandleScope handle_scope;
  if (line_template.IsEmpty()) {
    Handle<ObjectTemplate> raw_template = MakeLineTemplate();
    line_template = Persistent<ObjectTemplate>::New(raw_template);
  }
  Handle<Object> line = line_template->NewInstance();
  assert(line->InternalFieldCount() == 1);
  line->SetInternalField(0, External::New(this));
  return handle_scope.Close(line);
}
}
