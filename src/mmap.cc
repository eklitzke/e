// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./mmap.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef USE_LINUX
#define GET_OPEN_FLAGS(f)  (f | O_CLOEXEC)
#else
#define GET_OPEN_FLAGS(f)  (f)
#endif

namespace e {

MmapFile::MmapFile(const std::string &filename, bool writeable, bool sequential)
    :writeable_(writeable) {
  if (writeable) {
    fd_ = open(filename.c_str(), GET_OPEN_FLAGS(O_RDWR));
  } else {
    fd_ = open(filename.c_str(), GET_OPEN_FLAGS(O_RDONLY));
  }
  Initialize(sequential);
}

MmapFile::MmapFile(int fd, bool writeable, bool sequential)
    :writeable_(writeable), fd_(fd) {
  Initialize(sequential);
}

void MmapFile::Initialize(bool sequential) {
  if (fd_ == -1) {
    throw 1;  // FIXME(eklitzke)
  }
  struct stat sb;
  if (fstat(fd_, &sb) == -1) {
    close(fd_);
    throw 1;  // FIXME(eklitzke)
  }
  length_ = sb.st_size;

  int proto = PROT_READ;
  if (writeable_) {
    proto |= PROT_WRITE;
  }
  buffer_ = mmap(nullptr, length_, proto, MAP_PRIVATE, fd_, 0);
  if (sequential) {
    madvise(buffer_, length_, MADV_SEQUENTIAL | MADV_WILLNEED);
  }
}

MmapFile::~MmapFile() {
  close(fd_);
  if (writeable_) {
    msync(buffer_, length_, MS_SYNC);
  }
  munmap(buffer_, length_);
}

void* MmapFile::GetMapping() const {
  return buffer_;
}

size_t MmapFile::Size() const {
  return length_;
}
}
