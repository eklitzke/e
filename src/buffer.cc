// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

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
  lines_.push_back(Line());
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
    Line l(std::string(p, n - p));
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

std::vector<Line>* Buffer::Lines() {
  return &lines_;
}

namespace {
Handle<Value> JSGetLine(const Arguments& args) {
  GET_SELF(Buffer);
  if (args.Length() < 1) {
    return Undefined();
  }

  HandleScope scope;
  Handle<Value> arg0 = args[0];
  uint32_t offset = arg0->Uint32Value();
  std::vector<Line> l = *(self->Lines());
  assert(offset < l.size());
  return scope.Close(l[offset].ToScript());
}

Handle<Value> JSGetName(const Arguments& args) {
  GET_SELF(Buffer);

  HandleScope scope;
  std::string buffer_name = self->GetBufferName();
  return scope.Close(String::New(buffer_name.c_str(), buffer_name.length()));
}

Handle<Value> JSSize(const Arguments& args) {
  GET_SELF(Buffer);
  HandleScope scope;
  Local<Integer> size = Integer::New(self->Size());
  return scope.Close(size);
}

Persistent<ObjectTemplate> buffer_template;

// Create a raw template to assign to line_template
Handle<ObjectTemplate> MakeBufferTemplate() {
  HandleScope handle_scope;
  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->SetInternalFieldCount(1);
  result->Set(String::NewSymbol("getLine"), FunctionTemplate::New(JSGetLine),
    v8::ReadOnly);
  result->Set(String::NewSymbol("getName"), FunctionTemplate::New(JSGetName),
    v8::ReadOnly);
  result->Set(String::NewSymbol("size"), FunctionTemplate::New(JSSize),
    v8::ReadOnly);
  return handle_scope.Close(result);
}
}

Handle<Value> Buffer::ToScript() {
  HandleScope handle_scope;
  if (buffer_template.IsEmpty()) {
    Handle<ObjectTemplate> raw_template = MakeBufferTemplate();
    buffer_template = Persistent<ObjectTemplate>::New(raw_template);
  }
  Handle<Object> buf = buffer_template->NewInstance();
  assert(buf->InternalFieldCount() == 1);
  buf->SetInternalField(0, External::New(this));
  return handle_scope.Close(buf);
}
}
