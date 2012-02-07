#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "buffer.h"

namespace e {
  Buffer::Buffer (const std::string &name)
    :name_(name), c_line_(0), c_col_(0), window_top_(0)
  {
  }

  Buffer::Buffer (const std::string &name, const std::string &filepath)
    :filepath_(filepath), name_(name), c_line_(0), c_col_(0), window_top_(0)
  {
    int fd = open(filepath.c_str(), O_RDONLY);
    if (fd == -1) {
      throw 1;
    }
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
      throw 1;
    }

    char *mmaddr = static_cast<char *>(mmap(NULL, sb.st_size, PROT_READ,
                                            MAP_PRIVATE, fd, 0));
    madvise(mmaddr, sb.st_size, MADV_SEQUENTIAL);

    // read each line of the file into a new std::string, and store the string
    // into lines_
    char *p = mmaddr;
    while (p < mmaddr + sb.st_size) {
      char *n = static_cast<char *>(memchr(p, '\n', mmaddr + sb.st_size - p));
      std::string *s = new std::string(p, n - p);
      lines_.push_back(s);
      p = n + sizeof(char);
    }

    munmap(mmaddr, sb.st_size);
    close(fd);
  }

  std::vector<std::string *>*
  Buffer::get_lines(size_t start, size_t end) const
  {
    if (start >= lines_.size()) {
      return new std::vector<std::string *>(0);
    }

    size_t last_element = std::min(end, lines_.size());
    //std::vector<std::string *> *ret = new std::vector<std::string *>(last_element - start);
    std::vector<std::string *> *ret = new std::vector<std::string *>;
    for (size_t i = start; i < last_element; i++) {
      //ret[i - start] = lines_[i];
      ret->push_back(lines_[i]);
    }
    return ret;
  }

  std::vector<std::string *>*
  Buffer::get_lines(size_t num) const
  {
    return get_lines(window_top_, window_top_ + num);
  }


  std::string *
  Buffer::line_at(size_t index) const {
    return lines_[index];
  }

  size_t
  Buffer::num_lines() const {
    return lines_.size();
  }

  int
  Buffer::get_window_top() const
  {
    return window_top_;
  }

  const char *
  Buffer::get_name() const
  {
    return name_.c_str();
  }
}
