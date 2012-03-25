// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//

#include "./line.h"

#include <glog/logging.h>
#include <v8.h>

#include <algorithm>
#include <memory>
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

#define RETURN_SELF return scope.Close(self->ToV8String())

#ifndef TAB_SIZE
#define TAB_SIZE 4
#endif

namespace e {

void Line::ResetFromString(const std::string &str) {
  for (auto it = str.begin(); it != str.end(); ++it) {
    char c = *it;
    if (c == '\t') {
      for (size_t i = 0; i < TAB_SIZE; i++) {
        front_.push_back(static_cast<uint16_t>(' '));
      }
    } else {
      front_.push_back(static_cast<uint16_t>(c));
    }
  }
}

void Line::Replace(const std::string &str) {
  front_.clear();
  back_.clear();
  ResetFromString(str);
}

void Line::Refocus(const size_t position) const {
  ASSERT(position <= Size());
  const size_t current = front_.size();
  if (current > position) {
    for (size_t i = 0; i < current - position; i++) {
      back_.push_back(front_.back());
      front_.pop_back();
    }
  } else if (current < position) {
    for (size_t i = 0; i < position - current; i++) {
      front_.push_back(back_.back());
      back_.pop_back();
    }
  }
  ASSERT(front_.size() == position);
}

void Line::Chop(size_t new_length) {
  const size_t current_size = Size();
  if (current_size > new_length) {
    Refocus(Size());
    for (size_t i = 0; i < current_size - new_length; i++) {
      front_.pop_back();
    }
  }
}

void Line::Append(const uint16_t *buf, size_t length) {
  Refocus(Size());
  for (size_t i = 0; i < length; i++, buf++) {
    front_.push_back(*buf);
  }
}

void Line::Erase(size_t position, size_t count) {
  Refocus(position + count);
  for (size_t i = 0; i < count; i++) {
    front_.pop_back();
  }
}

void Line::ToBuffer(uint16_t *buf, bool refocus) const {
  if (refocus) {
    Refocus(0);
  }
  // copy the front
  if (front_.size()) {
    memcpy(static_cast<void *>(buf),
           static_cast<const void *>(front_.data()),
           front_.size() * sizeof(uint16_t));
  }
  // copy the back
  if (back_.size()) {
    std::vector<uint16_t> back_copy = back_;
    std::reverse(back_copy.begin(), back_copy.end());
    memcpy(static_cast<void *>(buf),
           static_cast<const void *>(back_copy.data()),
           back_copy.size() * sizeof(uint16_t));
  }
}

Local<String> Line::ToV8String(bool refocus) const {
  HandleScope scope;
  std::unique_ptr<uint16_t> buf(new uint16_t[Size()]);
  ToBuffer(buf.get(), refocus);
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

  // get the back part of the line as a JS string
  std::unique_ptr<uint16_t> buf(new uint16_t[self->Size()]);
  self->ToBuffer(buf.get());
  Local<String> back = String::New(buf.get() + offset * sizeof(uint16_t),
                                   static_cast<int>(self->Size() - offset));

  // chop the string
  self->Chop(static_cast<size_t>(offset));

  // return the chopped part
  return scope.Close(back);
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
  size_t position = static_cast<size_t>(arg0->Uint32Value());
  String::Utf8Value chars(arg1);

  for (int i = 0; i < chars.length(); i++) {
    self->InsertChar(position++, (*chars)[i]);
  }

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
