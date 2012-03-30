// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./list_environment.h"

#include <v8.h>
#include <cstdio>
#include <string>

#include "./js.h"

using v8::Array;
using v8::Context;
using v8::HandleScope;
using v8::Local;
using v8::Object;
using v8::String;

namespace e {
namespace {
static inline void Indent(int indentation) {
  if (indentation) {
    std::string padding(indentation, ' ');
    printf("%s", padding.c_str());
  }
}

void InspectValue(int indent, const std::string &name, Handle<Value> val) {
  Indent(indent);
  printf("%s", name.c_str());
  if (val->IsNumber()) {
    if (val->IsInt32()) {
      printf(" = %d\n", val->Int32Value());
    } else {
      printf(" = %f\n", val->NumberValue());
    }
  } else if (val->IsString()) {
    printf(" [String]\n");
  } else if (val->IsFunction()) {
    printf("()\n");
    Local<Object> obj = val->ToObject();
    Local<Array> props = obj->GetPropertyNames();
    for (uint32_t i = 0; i < props->Length(); i++) {
      Local<Value> name = props->Get(i);
      InspectValue(indent + 2, js::ValueToString(name), obj->Get(name));
    }

  } else if (val->IsObject()) {
    printf(" {\n");
    Local<Object> obj = val->ToObject();
    Local<Array> props = obj->GetPropertyNames();
    for (uint32_t i = 0; i < props->Length(); i++) {
      Local<Value> name = props->Get(i);
      InspectValue(indent + 2, js::ValueToString(name), obj->Get(name));
    }
    Indent(indent);
    printf("}\n");
  }
}
}

void ListEnvironment(Persistent<Context> context) {
  HandleScope scope;
  Local<Object> globals = context->Global();
  Local<Array> properties = globals->GetPropertyNames();

  for (uint32_t i = 0; i < properties->Length(); i++) {
    Local<Value> name = properties->Get(i);
    InspectValue(0, js::ValueToString(name), globals->Get(name));
  }
}
}
