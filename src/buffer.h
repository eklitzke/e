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

  // get a range of lines; caller owns the returned vector
  std::vector<std::string *>* get_lines(size_t, size_t) const;

  // get some number of lines, starting at the window top; caller owns the
  // returned vector
  std::vector<std::string *>* get_lines(size_t) const;

  // get the line at some offset (borrowed reference)
  std::string* line_at(size_t) const;

  // get the name of the buffer
  const std::string & get_name() const;

  // set the buffer name
  void set_name(const std::string &);

  // get the number of lines in the buffer
  size_t num_lines() const;

  // get the top line of the buffer
  int get_window_top() const;

  // is the buffer dirty?
  bool is_dirty(void) const;

  // get the line, col of the cursor
  void cursor_pos(int *, int *) const;

  // now the buffer owns the line
  void append_line(std::string *);
};
}

#endif  // BUFFER_H_
