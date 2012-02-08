// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// This file is AUTOGENERATED by gen_key_sources.py, do not edit by hand!

#ifndef KEYCODE_H_
#define KEYCODE_H_

#include <string>

namespace e {
  class KeyCode {
    private:
      int code_;
      std::string short_name_;
      std::string curses_macro_;
    public:
      explicit KeyCode(int code,
                       const std::string &short_name,
                       const std::string &curses_macro);
      explicit KeyCode(int code);
      const std::string& get_name(void) const;
      const std::string& get_curses_macro(void) const;
      bool is_ascii(void) const;
#if 0
      int get_code(void) const;
#endif
      char get_char(void) const;
  };

  namespace keycode {
    const KeyCode& curses_code_to_keycode(int, int*);
  }
}

#endif  // KEYCODE_H_