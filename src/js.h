// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_JS_H_
#define SRC_JS_H_

#include <v8.h>
#include <map>
#include <string>
#include <vector>

using v8::AccessorInfo;
using v8::Arguments;
using v8::Context;
using v8::External;
using v8::FunctionTemplate;
using v8::Handle;
using v8::InvocationCallback;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::Persistent;
using v8::String;
using v8::Value;

#define CHECK_ARGS(num) if (args.Length() < num) {  \
    return v8::Undefined();                         \
  }                                                 \
  HandleScope scope

#define NEW_INTEGER(obj, sym) obj->Set(v8::String::NewSymbol(#sym),     \
                                       v8::Integer::New(sym), v8::ReadOnly);

namespace e {
namespace js {

void AddTemplateFunction(Handle<ObjectTemplate>,
                         const std::string &,
                         v8::InvocationCallback);
void AddTemplateAccessor(Handle<ObjectTemplate>,
                         const std::string &,
                         v8::AccessorGetter,
                         v8::AccessorSetter);

typedef Handle<Value>(*JSCallback)(const Arguments&);
typedef Handle<Value>(*JSAccessor)(Local<String>, const AccessorInfo&);

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

// Reads a file into a v8 string.
Handle<v8::String> ReadFile(const std::string& name);
Handle<Value> JSAssert(const Arguments& args);
Handle<Value> JSLog(const Arguments& args);
Handle<Value> JSRequire(const Arguments& args);
std::string ValueToString(Local<Value>);

std::map<std::string, JSCallback> GetCallbacks();  // all of the callbacks
}
}

#endif  // SRC_JS_H_
