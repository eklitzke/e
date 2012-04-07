// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./timer.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <v8.h>

#include "./assert.h"
#include "./io_service.h"
#include "./js.h"

using v8::HandleScope;
using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::TryCatch;

namespace {
uint32_t next_id = 0;
std::map<uint32_t, e::Timer*> timers_;

void TimeoutHandler(e::Timer *timer, const boost::system::error_code& error) {
  if (!error && timer->Fire()) {
    delete timer;
  }
}
}

namespace e {
Timer::Timer(Persistent<Object> callback, uint32_t millis, bool repeat)
    :func_(callback), id_(next_id++), millis_(millis), repeat_(repeat),
     timer_(*GetIOService()) {
  ASSERT(func_->IsCallable());
  timers_.insert(timers_.end(), std::pair<uint32_t, Timer*>(id_, this));
}

#if 0
static Timer* New(Persistent<Object> callback, uint32_t millis, bool repeat) {
  return new Timer(callback, millis, repeat);
}
#endif

Timer::~Timer() {
  func_.Dispose();
  timer_.cancel();
  timers_.erase(id_);
}

void Timer::Start() {
  timer_.expires_from_now(boost::posix_time::milliseconds(millis_));
  timer_.async_wait(boost::bind(TimeoutHandler, this,
                                boost::asio::placeholders::error));
}

bool Timer::Fire() {
  HandleScope scope;
  TryCatch tr;
  Local<Object> this_argument = Object::New();
  func_->CallAsFunction(this_argument, 0, nullptr);
  HandleError(tr);
  if (repeat_) {
    timer_.expires_at(timer_.expires_at() +
                      boost::posix_time::milliseconds(millis_));
    timer_.async_wait(boost::bind(TimeoutHandler, this,
                                  boost::asio::placeholders::error));
    return false;
  } else {
    return true;
  }
}

size_t CancelAllTimers() {
  size_t deleted = 0;
  for (auto it = timers_.begin(); it != timers_.end(); ++it, deleted++) {
    delete it->second;
  }
  return deleted;
}

bool CancelTimerById(uint32_t id) {
  // cancel and delete a timer with some id
  auto it = timers_.find(id);
  if (it != timers_.end()) {
    delete it->second;
    return true;
  } else {
    return false;
  }
}
}
