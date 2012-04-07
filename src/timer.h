// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// This file implements the timers for setTimeout() and setInterval().

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include <boost/asio.hpp>
#include <v8.h>

using v8::Object;
using v8::Persistent;

namespace e {
class Timer {
 public:
  static Timer* New(Persistent<Object> callback, uint32_t millis, bool repeat) {
    return new Timer(callback, millis, repeat);
  }

  ~Timer();

  // start running the timer
  void Start();

  // fire the timer
  bool Fire();

  uint32_t GetId() const { return id_; }

 private:
  Timer(Persistent<Object> callback, uint32_t millis, bool repeat);

  Persistent<Object> func_;
  uint32_t id_;
  uint32_t millis_;
  bool repeat_;
  boost::asio::deadline_timer timer_;
};

// cancel and delete all timers
size_t CancelAllTimers();

// cancel a particular timer by id
bool CancelTimerById(uint32_t);
}

#endif  // SRC_TIMER_H_
