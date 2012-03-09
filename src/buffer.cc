// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <glog/logging.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <v8.h>

#include <algorithm>
#include <cassert>
#include <vector>

#include "./buffer.h"
#include "./js.h"

using v8::AccessorInfo;
using v8::Arguments;
using v8::Array;
using v8::Boolean;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Integer;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Undefined;
using v8::Value;

namespace e {
Buffer::Buffer(const std::string &name)
    :name_(name), dirty_(false) {
  Line *l = new Line();
  lines_.push_back(l);
}

Buffer::Buffer(const std::string &name, const std::string &filepath)
    :filepath_(filepath), name_(name), dirty_(false) {
  OpenFile(filepath);
}

void Buffer::OpenFile(const std::string &filepath) {
  int fd = open(filepath.c_str(), O_RDONLY);
  if (fd == -1) {
    throw 1; // FIXME(eklitzke)
  }
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    close(fd);
    throw 1; // FIXME(eklitzke)
  }

  // clear the old buffer
  for (auto it = lines_.begin(); it != lines_.end(); ++it) {
	delete *it;
  }
  lines_.clear();

  char *mmaddr = static_cast<char *>(mmap(nullptr, sb.st_size, PROT_READ,
                                          MAP_PRIVATE, fd, 0));
  madvise(mmaddr, sb.st_size, MADV_SEQUENTIAL);

  // read each line of the file into a new std::string, and store the string
  // into lines
  char *p = mmaddr;
  while (p < mmaddr + sb.st_size) {
    char *n = static_cast<char *>(memchr(p, '\n', mmaddr + sb.st_size - p));
    Line *l = new Line(std::string(p, n - p));
    lines_.push_back(l);
    p = n + sizeof(char);  // NOLINT
  }

  munmap(mmaddr, sb.st_size);
  close(fd);
}

size_t Buffer::Size() const {
  return lines_.size();
}

const std::string & Buffer::GetBufferName() const {
  return name_;
}

bool Buffer::IsDirty(void) const {
  return dirty_;
}

std::vector<Line*>* Buffer::Lines() {
  return &lines_;
}

namespace {
Handle<Value> JSAddLine(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Buffer);

  uint32_t offset = args[0]->Uint32Value();
  std::string lineValue;
  if (args.Length() >= 2) {
    String::AsciiValue value(args[1]);
    lineValue.insert(0, *value, value.length());
  }
  std::vector<Line *> *lines = self->Lines();
  assert(offset <= lines->size());

  Line *line = new Line(lineValue);
  if (offset == lines->size()) {
    lines->push_back(line);
  } else {
    lines->insert(lines->begin() + offset, line);
  }

  return scope.Close(line->ToScript());
}

Handle<Value> JSDeleteLine(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Buffer);

  uint32_t offset = args[0]->Uint32Value();
  std::vector<Line *> *lines = self->Lines();
  if (offset < lines->size()) {
    delete (*lines)[offset];
    lines->erase(lines->begin() + offset);
    return scope.Close(Boolean::New(true));
  } else {
    return scope.Close(Boolean::New(true));
  }
}

Handle<Value> JSGetLine(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Buffer);

  Handle<Value> arg0 = args[0];
  uint32_t offset = arg0->Uint32Value();
  std::vector<Line *> l = *(self->Lines());
  assert(offset < l.size());
  return scope.Close(l[offset]->ToScript());
}

// returns the buffer content as an array of strings
Handle<Value> JSGetContents(const Arguments& args) {
  GET_SELF(Buffer);
  HandleScope scope;
  const std::vector<Line *> l = *(self->Lines());
  Local<Array> arr = Array::New(l.size());
  for (size_t i = 0; i < l.size(); i++) {
    const std::string &s = l[i]->ToString();
    arr->Set(i, String::New(s.c_str(), s.size()));
  }
  return scope.Close(arr);
}

Handle<Value> JSGetName(const Arguments& args) {
  GET_SELF(Buffer);

  HandleScope scope;
  std::string buffer_name = self->GetBufferName();
  return scope.Close(String::New(buffer_name.c_str(), buffer_name.length()));
}

Handle<Value> JSGetLength(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  ACCESSOR_GET_SELF(Buffer);
  return scope.Close(Integer::New(self->Size()));
}

/*
void JSSetLength(Local<String> property, Local<Value> value,
               const AccessorInfo& info) {
  ACCESSOR_GET_SELF(Line);
  HandleScope scope;
  uint32_t newsize = value->Uint32Value();
  self->value.resize(static_cast<std::string::size_type>(newsize));
}
*/

Handle<Value> JSOpenFile(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Buffer);

  String::AsciiValue filename(args[0]);
  const std::string filename_s(*filename, filename.length());
  self->OpenFile(filename_s);
  return scope.Close(Undefined());
}

Persistent<ObjectTemplate> buffer_template;

// Create a raw template to assign to line_template
Handle<ObjectTemplate> MakeBufferTemplate() {
  HandleScope scope;
  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->SetInternalFieldCount(1);
  js::AddTemplateFunction(result, "addLine", JSAddLine);
  js::AddTemplateFunction(result, "deleteLine", JSDeleteLine);
  js::AddTemplateFunction(result, "getContents", JSGetContents);
  js::AddTemplateFunction(result, "getLine", JSGetLine);
  js::AddTemplateFunction(result, "getName", JSGetName);
  js::AddTemplateAccessor(result, "length", JSGetLength, nullptr);
  js::AddTemplateFunction(result, "open", JSOpenFile);
  return scope.Close(result);
}
}

Handle<Value> Buffer::ToScript() {
  HandleScope scope;
  if (buffer_template.IsEmpty()) {
    Handle<ObjectTemplate> raw_template = MakeBufferTemplate();
    buffer_template = Persistent<ObjectTemplate>::New(raw_template);
  }
  Handle<Object> buf = buffer_template->NewInstance();
  assert(buf->InternalFieldCount() == 1);
  buf->SetInternalField(0, External::New(this));
  return scope.Close(buf);
}
}
