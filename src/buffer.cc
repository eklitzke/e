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
#include <vector>

#include "./buffer.h"

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

namespace e {
Buffer::Buffer(const std::string &name)
    :name_(name), dirty_(false) {
  Line *l = new Line();
  lines_.push_back(l);
}

Buffer::Buffer(const std::string &name, const std::string &filepath)
    :filepath_(filepath), name_(name), dirty_(false) {
  int fd = open(filepath.c_str(), O_RDONLY);
  if (fd == -1) {
    throw 1;
  }
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    close(fd);
    throw 1;
  }

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
  GET_SELF(Buffer);
  if (args.Length() < 1) {
    return Undefined();
  }

  HandleScope scope;
  Handle<Value> arg0 = args[0];
  uint32_t offset = arg0->Uint32Value();
  std::vector<Line *> *lines = self->Lines();
  assert(offset <= lines->size());

  Line *line = new Line();
  if (offset == lines->size()) {
    LOG(INFO) << "addline, push_back";
    lines->push_back(line);
    LOG(INFO) << "addline, push_back, size is now" << lines->size();
  } else {
    LOG(INFO) << "addline, insert";
    lines->insert(lines->begin() + offset, line);
  }

  return scope.Close(line->ToScript());
}

Handle<Value> JSGetLine(const Arguments& args) {
  GET_SELF(Buffer);
  if (args.Length() < 1) {
    return Undefined();
  }

  HandleScope scope;
  Handle<Value> arg0 = args[0];
  uint32_t offset = arg0->Uint32Value();
  std::vector<Line *> l = *(self->Lines());
  LOG(INFO) << "offset is " << offset << ", size is " << l.size();
  assert(offset < l.size());
  return scope.Close(l[offset]->ToScript());
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

Persistent<ObjectTemplate> buffer_template;

// Create a raw template to assign to line_template
Handle<ObjectTemplate> MakeBufferTemplate() {
  HandleScope scope;
  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->SetInternalFieldCount(1);
  result->Set(String::NewSymbol("addLine"), FunctionTemplate::New(JSAddLine),
    v8::ReadOnly);
  result->Set(String::NewSymbol("getLine"), FunctionTemplate::New(JSGetLine),
    v8::ReadOnly);
  result->Set(String::NewSymbol("getName"), FunctionTemplate::New(JSGetName),
    v8::ReadOnly);
  result->SetAccessor(String::NewSymbol("length"), JSGetLength);
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
