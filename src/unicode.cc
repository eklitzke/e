// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./unicode.h"

#include <unicode/unistr.h>
#include <stdint.h>
#include <v8.h>

#include <memory>

using v8::HandleScope;
using v8::Local;
using v8::String;

namespace e {
Local<String> UnicodeToString(const UnicodeString &str) {
  HandleScope scope;
  int length = str.length();
  std::unique_ptr<UChar> data(new UChar[length]);
  UErrorCode err;
  str.extract(data.get(), length, err);
  Local<String> js_string = String::New(
          static_cast<const uint16_t*>(data.get()), length);
  return scope.Close(js_string);
}
}
