// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//

#include "./line.h"

#include <glog/logging.h>
#include <v8.h>

#include <string>
#include <vector>

#include "./assert.h"
#include "./embeddable.h"
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

#define RETURN_SELF return scope.Close(\
    String::New(self->value.c_str(), self->value.size()))

#ifndef TAB_SIZE
#define TAB_SIZE 4
#endif

namespace e {

Line::Line(const std::string &line) {
  std::string::size_type lo = 0;
  std::string::size_type hi = 0;
  while (true) {
    hi = line.find('\t', lo);
    if (hi == std::string::npos) {
      value += line.substr(lo, hi);
      break;
    } else {
      if (lo != hi) {
        value += line.substr(lo, hi - lo);
      }
      value += std::string(TAB_SIZE, ' ');
    }
    lo = hi + 1;
  }
}

void Line::Replace(const std::string &new_line) {
  value = new_line;
#if 0
  for (auto it = callbacks_.begin(); it != callbacks_.end(); ++it) {
    (*it)(new_line);
  }
#endif
}

const std::string& Line::ToString() const {
  return value;
}

#if 0
void Line::OnChange(StringCallback cb) {
  callbacks_.push_back(cb);
}
#endif

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

  String::AsciiValue value(args[0]);
  self->value.append(*value, value.length());
  RETURN_SELF;
}

// @method: chop
// @param[offset]: #int the offset to chop at
// @description: Chops from the offset to the end of the line. Returns the
//               chopped section as a JavaScript string.
Handle<Value> JSChop(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Line);

  uint32_t offset = args[0]->Uint32Value();
  std::string chopped = self->value.substr(offset, std::string::npos);
  self->value.erase(self->value.begin() + offset, self->value.end());
  return scope.Close(String::New(chopped.c_str(), chopped.size()));
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
  uint32_t offset = arg0->Uint32Value();
  uint32_t amt = arg1->Uint32Value();
  self->value.erase(static_cast<size_t>(offset), static_cast<size_t>(amt));
  RETURN_SELF;
}

// @method: insert
// @param[offset]: #int the offset to insert at
// @param[str]: #string the string to insert
// @description: Inserts characters in the middle of the line. Returns the new
//               Line.
Handle<Value> JSInsert(const Arguments& args) {
  CHECK_ARGS(2);
  GET_SELF(Line);
  Handle<Value> arg0 = args[0];
  Handle<Value> arg1 = args[1];
  uint32_t position = arg0->Uint32Value();
  String::Utf8Value chars(arg1);

  self->value.insert(static_cast<size_t>(position), *chars, chars.length());
  RETURN_SELF;
}

// @method: value
// @description: Returns the contents of the line as a JavaScript string.
Handle<Value> JSValue(const Arguments& args) {
  HandleScope scope;
  GET_SELF(Line);
  RETURN_SELF;
}

// @accessor: length
// @description: Returns the length of the line.
Handle<Value> JSGetLength(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  ACCESSOR_GET_SELF(Line);
  return scope.Close(Integer::New(self->value.length()));
}

void JSSetLength(Local<String> property, Local<Value> value,
               const AccessorInfo& info) {
  ACCESSOR_GET_SELF(Line);
  HandleScope scope;
  uint32_t newsize = value->Uint32Value();
  self->value.resize(static_cast<std::string::size_type>(newsize));
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

Handle<Value> Line::ToScript() {
  HandleScope scope;
  if (line_template.IsEmpty()) {
    Handle<ObjectTemplate> raw_template = MakeLineTemplate();
    line_template = Persistent<ObjectTemplate>::New(raw_template);
  }
  Handle<Object> line = line_template->NewInstance();
  ASSERT(line->InternalFieldCount() == 1);
  line->SetInternalField(0, External::New(this));
  return scope.Close(line);
}
}
