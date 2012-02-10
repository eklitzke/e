// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include <map>
#include <string>
#include <vector>

#include "./js.h"

using namespace v8;

namespace e {
  namespace js {

    void
    EventListener::add(const std::string& callback_name,
                       Persisent<Object *> callback,
                       bool use_capture)
    {

      std::vector<Persisent<Object *> > &callback_map;
      if (use_capture)
        callback_map = capture_;
      else
        callback_map = bubble_;

      // if callback is already registered, do nothing


    class EventListener {
    public:
      void add(const std::string&, Persisent<Function *>);
      void remove(const std::string&, Persisent<Function *>);
      void dispatch(const std::string&, const v8::Arguments&);
    private:
      std::map<std::string, std::vector<v8::Persisent<v8::Function *> > capture_;
      std::map<std::string, std::vector<v8::Persisent<v8::Function *> > bubble_;
    };
  }
}

#endif  // JS_H_
