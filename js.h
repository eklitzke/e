// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef JS_H_
#define JS_H_

#include <v8.h>
#include <map>
#include <string>
#include <vector>

namespace e {
  namespace js {
    class EventListener {
    public:
      void add(const std::string&, v8::Persisent<Object *>, bool);
      void remove(const std::string&, v8::Persisent<Object *>, bool);
      void dispatch(const std::string&, const v8::Arguments&);
    private:
      std::map<std::string, std::vector<v8::Persisent<v8::Object *> > capture_;
      std::map<std::string, std::vector<v8::Persisent<v8::Object *> > bubble_;
    };
  }
}

#endif  // JS_H_
