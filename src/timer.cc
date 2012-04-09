// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./timer.h"

#include <boost/asio.hpp>
#include <v8.h>

#include <functional>

#include "./assert.h"
#include "./io_service.h"
#include "./js.h"

using v8::Boolean;
using v8::HandleScope;
using v8::Integer;
using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::TryCatch;
using v8::Undefined;

namespace {
uint32_t next_id = 0;
std::map<uint32_t, e::Timer*> timers_;

void TimeoutHandler(e::Timer *timer, const boost::system::error_code& error) {
  if (!error && timer->Fire()) {
    delete timer;
  }
}

// @class: timers (not really)
//
// @method: setTimeout
// @description: run code after a timeout
Handle<Value> JSSetTimeout(const Arguments& args) {
  CHECK_ARGS(2);
  Handle<Value> val = args[0];
  uint32_t millis = args[1]->Uint32Value();

  Local<Object> obj = val->ToObject();
  if (obj->IsCallable()) {
    Persistent<Object> func = Persistent<Object>::New(obj);
    e::Timer *timer = e::Timer::New(func, millis, false);
    timer->Start();
    return scope.Close(Integer::New(timer->GetId()));
  } else {
    return Undefined();
  }
}

// @method: setInterval
// @description: run code in an interval
Handle<Value> JSSetInterval(const Arguments& args) {
  CHECK_ARGS(2);
  Handle<Value> val = args[0];
  uint32_t millis = args[1]->Uint32Value();

  Local<Object> obj = val->ToObject();
  if (obj->IsCallable()) {
    Persistent<Object> func = Persistent<Object>::New(obj);
    e::Timer *timer = e::Timer::New(func, millis, true);
    timer->Start();
    return scope.Close(Integer::New(timer->GetId()));
  } else {
    return Undefined();
  }
}

// @method: clearTimeout
// @description: cancel a timeout created by setTimeout()
// @method: clearInterval
// @description: cancel a timeout created by setInterval()
//
// N.B. this method works for both timeouts and intervals
Handle<Value> JSClearTimeout(const Arguments& args) {
  CHECK_ARGS(1);
  uint32_t id = args[0]->Uint32Value();
  auto it = timers_.find(id);
  if (it != timers_.end()) {
    delete it->second;
    return scope.Close(Boolean::New(true));
  } else {
    return scope.Close(Boolean::New(false));
  }
}
}

namespace e {
Timer::Timer(Persistent<Object> callback, uint32_t millis, bool repeat)
    :func_(callback), id_(next_id++), millis_(millis), repeat_(repeat),
     timer_(io_service) {
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
  timer_.async_wait(std::bind(TimeoutHandler, this,
                              std::placeholders::_1));
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
    timer_.async_wait(std::bind(TimeoutHandler, this,
                                std::placeholders::_1));
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

void AddTimersToGlobalNamespace(Local<ObjectTemplate> global) {
  global->Set(String::NewSymbol("setTimeout"),
              FunctionTemplate::New(JSSetTimeout), v8::ReadOnly);
  global->Set(String::NewSymbol("setInterval"),
              FunctionTemplate::New(JSSetInterval), v8::ReadOnly);
  global->Set(String::NewSymbol("clearTimeout"),
              FunctionTemplate::New(JSClearTimeout), v8::ReadOnly);
  global->Set(String::NewSymbol("clearInterval"),
              FunctionTemplate::New(JSClearTimeout), v8::ReadOnly);
}
}
