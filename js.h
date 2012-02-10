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
      bool add(const std::string&, v8::Handle<v8::Object> *, bool);
      bool remove(const std::string&, v8::Handle<v8::Object> *, bool);
      void dispatch(const std::string&, const v8::Arguments&);
    private:
      std::map<std::string, std::vector<v8::Handle<v8::Object>*> > capture_;
      std::map<std::string, std::vector<v8::Handle<v8::Object>*> > bubble_;

	  std::vector<v8::Handle<v8::Object>*> & callback_map(const std::string &, bool);
    };
  }
}

#endif  // JS_H_
