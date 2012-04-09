// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//

#include "./line.h"

#include <v8.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "./assert.h"
#include "./embeddable.h"
#include "./logging.h"
#include "./js.h"

using v8::AccessorInfo;
using v8::Arguments;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Integer;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Undefined;
using v8::Value;

#ifndef TAB_SIZE
#define TAB_SIZE 4
#endif

namespace e {

void Line::Replace(const std::string& newline) {
  std::vector<uint16_t> data;
  for (auto it = newline.begin(); it != newline.end(); ++it) {
    data.push_back(static_cast<uint16_t>(*it));
  }
  zipper_.Clear();
  zipper_.Append(data.data(), newline.size());
}

Local<String> Line::ToV8String(bool refocus) const {
  HandleScope scope;
  std::unique_ptr<uint16_t> buf(new uint16_t[Size()]);
  zipper_.ToBuffer(buf.get(), refocus);
  return scope.Close(String::New(buf.get(),
                                 static_cast<int>(Size())));
}

std::string Line::ToString(bool refocus) const {
  std::string str;
  HandleScope scope;
  Local<String> jstr = ToV8String(refocus);
  if (jstr->Utf8Length()) {
    std::unique_ptr<char> buf(new char[jstr->Utf8Length() + 1]);
    jstr->WriteUtf8(buf.get(), jstr->Utf8Length());
    buf.get()[jstr->Utf8Length()] = '\0';
    str = buf.get();
  }
  return str;
}

namespace {
// @class: Line
// @description: This class is the internal representation of a line of text,
//               and is the subunit of a `Buffer.. It is similar to a JavaScript
//               string, and can be converted to and from one.

// @method: append
// @param[str]: #string the string to append
// @description: Appends the string to the line. Returns the new Line.
Handle<Value> JSAppend(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Line);

  String::Value value(args[0]);
  self->Append(*value, static_cast<size_t>(value.length()));
  return scope.Close(Undefined());
}

// @method: chop
// @param[offset]: #int the offset to chop at
// @description: Chops from the offset to the end of the line.
Handle<Value> JSChop(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Line);
  uint32_t offset = args[0]->Uint32Value();
  self->Chop(static_cast<size_t>(offset));
  return scope.Close(Undefined());
}

// @method: erase
// @param[offset]: #int the offset to erase at
// @param[num_chars]: #int the number of characters to erase
// @description: Erases characters from the middle of the line. Returns the new
//               Line.
Handle<Value> JSErase(const Arguments& args) {
  CHECK_ARGS(2);
  GET_SELF(Line);
  Handle<Value> arg0 = args[0];
  Handle<Value> arg1 = args[1];
  size_t offset = static_cast<size_t>(arg0->Uint32Value());
  size_t amt = static_cast<size_t>(arg1->Uint32Value());
  self->Erase(offset, amt);
  return scope.Close(Undefined());
}

// @method: insert
// @param[offset]: #int the offset to insert at
// @param[str]: #string the string to insert
// @description: Inserts characters in the middle of the line.
Handle<Value> JSInsert(const Arguments& args) {
  CHECK_ARGS(2);
  GET_SELF(Line);
  Handle<Value> arg0 = args[0];
  Handle<Value> arg1 = args[1];
  size_t position = static_cast<size_t>(arg0->Uint32Value());
  String::Utf8Value chars(arg1);

  for (int i = 0; i < chars.length(); i++) {
    self->InsertChar(position++, (*chars)[i]);
  }

  return scope.Close(Undefined());
}

// @method: value
// @param[refocus]: #bool whether to refocus (optional), defaults true
// @description: Returns the contents of the line as a JavaScript string.
Handle<Value> JSValue(const Arguments& args) {
  HandleScope scope;
  GET_SELF(Line);
  bool refocus = true;
  if (args.Length() >= 1) {
    refocus = args[0]->ToBoolean()->Value();
  }
  return scope.Close(self->ToV8String(refocus));
}

// @accessor: length
// @description: Returns the length of the line.
Handle<Value> JSGetLength(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  ACCESSOR_GET_SELF(Line);
  return scope.Close(Integer::New(self->Size()));
}

void JSSetLength(Local<String> property, Local<Value> value,
               const AccessorInfo& info) {
  ACCESSOR_GET_SELF(Line);
  HandleScope scope;
  uint32_t newsize = value->Uint32Value();
  self->Chop(static_cast<size_t>(newsize));
}

Persistent<ObjectTemplate> line_template;

// Create a raw template to assign to line_template
Handle<ObjectTemplate> MakeLineTemplate() {
  HandleScope handle_scope;
  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->SetInternalFieldCount(1);
  js::AddTemplateFunction(result, "append", JSAppend);
  js::AddTemplateFunction(result, "chop", JSChop);
  js::AddTemplateFunction(result, "erase", JSErase);
  js::AddTemplateFunction(result, "insert", JSInsert);
  js::AddTemplateFunction(result, "value", JSValue);
  js::AddTemplateAccessor(result, "length", JSGetLength, JSSetLength);
  return handle_scope.Close(result);
}
}

Local<Value> Line::ToScript() {
  HandleScope scope;
  if (line_template.IsEmpty()) {
    Handle<ObjectTemplate> raw_template = MakeLineTemplate();
    line_template = Persistent<ObjectTemplate>::New(raw_template);
  }
  Local<Object> line = line_template->NewInstance();
  ASSERT(line->InternalFieldCount() == 1);
  line->SetInternalField(0, External::New(this));
  return scope.Close(line);
}
}
