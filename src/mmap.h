// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_MMAP_H_
#define SRC_MMAP_H_

#include <string>

namespace e {
class MmapFile {
 public:
  explicit MmapFile(const std::string &filename, bool writeable = false,
                    bool sequential = true);
  explicit MmapFile(const int fd, bool writeable = false,
                    bool sequential = true);
  ~MmapFile();
  void* GetMapping() const;
  size_t Size() const;
 private:
  bool writeable_;
  int fd_;
  size_t length_;
  void* buffer_;

  void Initialize(bool sequential);
};
}

#endif  // SRC_MMAP_H_
