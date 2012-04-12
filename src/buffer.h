// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_BUFFER_H_
#define SRC_BUFFER_H_

#include <v8.h>

#include <string>
#include <vector>

#include "./line.h"

using v8::Handle;
using v8::Value;

namespace e {
class Buffer {
 public:
  // constructors
  explicit Buffer(const std::string &name, bool scratch = true);
  explicit Buffer(const std::string &name, const std::string &filepath);
  ~Buffer();

  bool OpenFile(const std::string &filepath);
  void Persist(const std::string &filepath);  // write the buffer to disk

  // get the name of the buffer
  const std::string & GetBufferName() const;

  // get the name of the filepath
  const std::string & GetFilePath() const;

  // set the buffer name
  void SetBufferName(const std::string &);

  // get the number of lines in the buffer
  inline size_t Size() const { return lines_.Size(); }

  // append a line to the buffer
  inline void AppendLine(const std::string &s) {
    Line *l = new Line(s);
    lines_.Insert(Size(), l);
  }

  inline Line* operator[](size_t offset) { return lines_[offset]; }

  // insert a line at some offset
  Line* Insert(size_t, const std::string &);

  // erase a line at some offset
  void Erase(size_t);

  // is this a scratch buffer?
  bool IsScratch() { return scratch_; }

  Handle<Value> ToScript();

 private:
  std::string filepath_;
  std::string name_;
  bool scratch_;
  Zipper<Line *> lines_;
};
}

#endif  // SRC_BUFFER_H_
