// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_BUFFER_H_
#define SRC_BUFFER_H_

#include <v8.h>

#include <string>
#include <vector>

#include "./embeddable.h"
#include "./line.h"

using v8::Handle;
using v8::Value;

namespace e {
class Buffer: public Embeddable {
 private:
  std::string filepath_;
  std::string name_;

  bool dirty_;

  std::vector<Line *> lines_;

 public:
  // constructors
  explicit Buffer(const std::string &name);
  explicit Buffer(const std::string &name, const std::string &filepath);

  void OpenFile(const std::string &filepath);
  void Persist(const std::string &filepath);  // write the buffer to disk

  // get the name of the buffer
  const std::string & GetBufferName() const;

  // get the name of the filepath
  const std::string & GetFilePath() const;

  // set the buffer name
  void SetBufferName(const std::string &);

  // get the number of lines in the buffer
  size_t Size() const;

  std::vector<Line*>* Lines();

  // is the buffer dirty?
  bool IsDirty(void) const;

  Handle<Value> ToScript();
};
}

#endif  // SRC_BUFFER_H_
