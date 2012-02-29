// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <vector>

#include "./buffer.h"

namespace e {
Buffer::Buffer(const std::string &name)
    :name_(name), dirty_(false) {
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

size_t
Buffer::Size() const {
  return lines_.size();
}

const std::string &
Buffer::GetBufferName() const {
  return name_;
}

bool
Buffer::IsDirty(void) const {
  return dirty_;
}
}
