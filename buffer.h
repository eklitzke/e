// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef BUFFER_H_
#define BUFFER_H_

#include <string>
#include <vector>

namespace e {
  class Buffer {
  private:
    std::vector<std::string *> lines_;
    std::string filepath_;
    std::string name_;

    // cursor line and column
    int c_line_;
    int c_col_;

    // top line shown in window
    int window_top_;

    bool dirty_;

  public:
    // constructors
    explicit Buffer(const std::string &name);
    explicit Buffer(const std::string &name, const std::string &filepath);

    std::vector<std::string *>* get_lines(size_t start, size_t end) const;
    std::vector<std::string *>* get_lines(size_t num) const;

    std::string* line_at(size_t index) const;

    const std::string & get_name() const;
    void set_name(const std::string &);
    size_t num_lines() const;

    int get_window_top() const;
    bool is_dirty(void) const;

    void cursor_pos(int *, int *) const;
  };
}

#endif  // BUFFER_H_
