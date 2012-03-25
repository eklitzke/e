// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <glog/logging.h>

#include <string.h>
#include <unistd.h>
#include <v8.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "./assert.h"
#include "./buffer.h"
#include "./js.h"
#include "./mmap.h"

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
  MmapFile mapping(filepath);

  // clear the old buffer
  for (auto it = lines_.begin(); it != lines_.end(); ++it) {
    delete *it;
  }
  lines_.clear();

  // read each line of the file into a new std::string, and store the string
  // into lines
  char *mmaddr = static_cast<char *>(mapping.GetMapping());
  const size_t mmlen = mapping.Size();
  char *p = mmaddr;
  while (p < mmaddr + mmlen) {
    char *n = static_cast<char *>(memchr(p, '\n', mmaddr + mmlen - p));
    Line *l = new Line(std::string(p, n - p));
    lines_.push_back(l);
    p = n + sizeof(char);  // NOLINT
  }

  filepath_ = filepath;
  name_ = filepath;
}

// FIXME: we definitely need more robust error handling
void Buffer::Persist(const std::string &filepath) {
#if 0
  std::string tmp_template;
  if (getenv("TEMPDIR") != nullptr) {
    tmp_template = getenv("TEMPDIR");
  } else {
    tmp_template = "/tmp";
  }
  while (tmp_template.length() &&
         tmp_template[tmp_template.length() - 1] == '/') {
    tmp_template.pop_back();
  }
  tmp_template += "/.e-XXXXXX~";
#else
  std::string tmp_template = "./.e-XXXXXX~";
#endif

  std::unique_ptr<char> filename(new char[tmp_template.length()]);
  memcpy(filename.get(), tmp_template.c_str(), tmp_template.length());
  filename.get()[tmp_template.length()] = '\0';
  int fd = mkstemps(filename.get(), 1);
  ASSERT(fd >= 0);

  for (auto it = lines_.begin(); it != lines_.end(); ++it) {
    std::string line = (*it)->ToString();
    size_t written = 0;
    while (written < line.length()) {
      ssize_t w = write(fd, line.c_str() + written, line.length() - written);
      ASSERT(w >= 0);
      written += w;
    }
    ASSERT(write(fd, "\n", 1) == 1);
  }
  ASSERT(fsync(fd) == 0);
  ASSERT(rename(filename.get(), filepath.c_str()) == 0);
  ASSERT(close(fd) == 0);
}

size_t Buffer::Size() const {
  return lines_.size();
}

const std::string & Buffer::GetBufferName() const {
  return name_;
}


const std::string & Buffer::GetFilePath() const {
  return filepath_;
}

bool Buffer::IsDirty(void) const {
  return dirty_;
}

std::vector<Line*>* Buffer::Lines() {
  return &lines_;
}

namespace {
// @class: Buffer
// @description: The internal representation of a buffer.
//
// @method: addLine
// @param[offset]: #int line number for the newly inserted line
// @description: Adds a line to the buffer.
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
  ASSERT(offset <= lines->size());

  Line *line = new Line(lineValue);
  if (offset == lines->size()) {
    lines->push_back(line);
  } else {
    lines->insert(lines->begin() + offset, line);
  }

  return scope.Close(line->ToScript());
}

// @method: deleteLine
// @param[offset]: #int line number of the line to delete
// @description: Removes a line from the buffer; returns true if the line was
//               deleted, false otherwise.
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
    return scope.Close(Boolean::New(false));
  }
}

// @method: getLine
// @param[offset]: #int line number of the line to get
// @description: Gets a Line object from the buffer.
Handle<Value> JSGetLine(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Buffer);

  Handle<Value> arg0 = args[0];
  uint32_t offset = arg0->Uint32Value();
  std::vector<Line *> l = *(self->Lines());
  ASSERT(offset < l.size());
  return scope.Close(l[offset]->ToScript());
}

// @method: getContents
// @description: Returns the buffer as an array of strings.
Handle<Value> JSGetContents(const Arguments& args) {
  GET_SELF(Buffer);
  HandleScope scope;
  const std::vector<Line *> l = *(self->Lines());
  Local<Array> arr = Array::New(l.size());
  for (size_t i = 0; i < l.size(); i++) {
    std::string s = l[i]->ToString();
    arr->Set(i, String::New(s.c_str(), s.size()));
  }
  return scope.Close(arr);
}

// @method: getFile
// @description: Returns the name of the file backing the buffer.
Handle<Value> JSGetFile(const Arguments& args) {
  GET_SELF(Buffer);

  HandleScope scope;
  std::string buffer_name = self->GetFilePath();
  return scope.Close(String::New(buffer_name.c_str(), buffer_name.length()));
}

// @method: getName
// @description: Returns the name of the buffer.
Handle<Value> JSGetName(const Arguments& args) {
  GET_SELF(Buffer);

  HandleScope scope;
  std::string buffer_name = self->GetBufferName();
  return scope.Close(String::New(buffer_name.c_str(), buffer_name.length()));
}

// @accessor: length
// @description: Returns the number of lines in the buffer.
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

// @method: open
// @param[filename]: #string The name of the file to open.
// @description: Open a file (this method blocks).
Handle<Value> JSOpenFile(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Buffer);

  String::AsciiValue filename(args[0]);
  const std::string filename_s(*filename, filename.length());
  self->OpenFile(filename_s);
  return scope.Close(Undefined());
}

// @method: persist
// @param[filename]: #string The name of the file to write to.
// @description: Persist the buffer contents to a file (this method blocks).
Handle<Value> JSPersist(const Arguments& args) {
  CHECK_ARGS(1);
  GET_SELF(Buffer);

  String::AsciiValue filename(args[0]);
  const std::string filename_s(*filename, filename.length());
  self->Persist(filename_s);
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
  js::AddTemplateFunction(result, "getFile", JSGetFile);
  js::AddTemplateFunction(result, "getLine", JSGetLine);
  js::AddTemplateFunction(result, "getName", JSGetName);
  js::AddTemplateAccessor(result, "length", JSGetLength, nullptr);
  js::AddTemplateFunction(result, "open", JSOpenFile);
  js::AddTemplateFunction(result, "persist", JSPersist);
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
  ASSERT(buf->InternalFieldCount() == 1);
  buf->SetInternalField(0, External::New(this));
  return scope.Close(buf);
}
}
