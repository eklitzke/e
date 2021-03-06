#!/usr/bin/env python

import datetime
import optparse
import os.path
import re
import sys

h_template = """
// -*- C++ -*-
// Copyright %(current_year)d, Evan Klitzke <evan@eklitzke.org>
//
// This file is AUTOGENERATED by gen_key_sources.py, do not edit by hand!

#ifndef SRC_KEYCODE_H_
#define SRC_KEYCODE_H_

#include <ctype.h>
#include <v8.h>

#include <string>

using v8::Arguments;
using v8::Handle;
using v8::Object;
using v8::Persistent;
using v8::Value;

namespace e {
class KeyCode {
 public:
  explicit KeyCode(wint_t code);
  explicit KeyCode(wint_t code, const std::string &name);
  ~KeyCode();
  Persistent<Value> ToScript();
  inline wint_t Code() const { return code_; }
  inline bool IsKeypad() const { return is_keypad_; }
  inline const std::string& Name() const { return name_; }
  inline bool IsPrintable() const {
     return static_cast<bool>(isprint(static_cast<int>(code_)));
  }
 private:
  wint_t code_;
  bool is_keypad_;
  std::string name_;
};

KeyCode* CursesToKeycode(const wint_t &wch, bool is_keypad);
Handle<Object> GetKeycodeMap();
}

#endif  // SRC_KEYCODE_H_
"""

cc_template = """
// -*- C++ -*-
// Copyright %(current_year)d, Evan Klitzke <evan@eklitzke.org>
//
// This file is AUTOGENERATED by gen_key_sources.py, do not edit by hand!

#include "./%(h_name)s"

#include <v8.h>
#include <wchar.h>

#include <string>

#include "./assert.h"
#include "./embeddable.h"

using v8::Arguments;
using v8::Boolean;
using v8::External;
using v8::FunctionTemplate;
using v8::Handle;
using v8::HandleScope;
using v8::Integer;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Undefined;
using v8::Value;

namespace e {
namespace {
// @class: KeyCode
// @description: Internal representation of a keypress.

// @method: getChar
// @description: Returns the JavaScript string representation of a keypress;
//               this will be a string of length 1 that will present a single
//               unicode character.
Handle<Value> JSGetChar(const Arguments& args) {
  HandleScope scope;
  KeyCode *self = Unwrap<KeyCode>(args);

  ASSERT(self->Code() < 0x10000);
  uint16_t c = static_cast<uint16_t>(self->Code());
  return scope.Close(String::New(&c, 1));
}

// @method: getCode
// @description: Returns the integral code for a character (generally its
//               Unicode codepoint).
Handle<Value> JSGetCode(const Arguments& args) {
  HandleScope scope;
  KeyCode *self = Unwrap<KeyCode>(args);
  Local<Integer> code = Integer::New(self->Code());
  return scope.Close(code);
}

// @method: getName
// @description: Returns the name for a keypress; this will be an empty string
//               unless the keycode represents a keypad hit
Handle<Value> JSGetName(const Arguments& args) {
  HandleScope scope;
  KeyCode *self = Unwrap<KeyCode>(args);
  if (self->IsKeypad()) {
    const std::string &s = self->Name();
    Local<String> name = String::NewSymbol(s.c_str(), s.length());
    return scope.Close(name);
  } else {
    return scope.Close(String::Empty());
  }
}

// @method: isKeypad
// @description: Returns true if this was a keypad hit, false otherwise.
Handle<Value> JSIsKeypad(const Arguments& args) {
  HandleScope scope;
  KeyCode *self = Unwrap<KeyCode>(args);
  Handle<Boolean> b = Boolean::New(self->IsKeypad());
  return scope.Close(b);
}

// @method: isPrintable
// @description: Returns true if this char is printable, false otherwise.
Handle<Value> JSIsPrintable(const Arguments& args) {
  HandleScope scope;
  KeyCode *self = Unwrap<KeyCode>(args);
  bool printable = !self->IsKeypad() && self->IsPrintable();
  Handle<Boolean> b = Boolean::New(printable);
  return scope.Close(b);
}

Persistent<ObjectTemplate> keycode_template;

// Create a raw template to assign to keycode_template
Handle<ObjectTemplate> MakeKeyCodeTemplate() {
  HandleScope scope;
  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->SetInternalFieldCount(1);
  result->Set(String::New("getChar"), FunctionTemplate::New(JSGetChar),
    v8::ReadOnly);
  result->Set(String::New("getCode"), FunctionTemplate::New(JSGetCode),
    v8::ReadOnly);
  result->Set(String::New("getName"), FunctionTemplate::New(JSGetName),
    v8::ReadOnly);
  result->Set(String::New("isPrintable"), FunctionTemplate::New(JSIsPrintable),
    v8::ReadOnly);
  result->Set(String::New("isKeypad"), FunctionTemplate::New(JSIsKeypad),
    v8::ReadOnly);
  return scope.Close(result);
}
}

KeyCode::KeyCode(wint_t code, const std::string &name)
    :code_(code), is_keypad_(true), name_(name) {
}

KeyCode::KeyCode(wint_t code)
    :code_(code), is_keypad_(false) {
}

KeyCode::~KeyCode() {
}

namespace {
// this callback will be invoked when the V8 keypress object is GC'ed
void CleanupKeycode(Persistent<Value> val, void*) {
  HandleScope scope;
  ASSERT(val->IsObject());
  Local<Object> obj = val->ToObject();
  KeyCode *kc = UnwrapObj<KeyCode>(obj);
  delete kc;
  val.Dispose();
}
}

Persistent<Value> KeyCode::ToScript() {
  HandleScope scope;

  if (keycode_template.IsEmpty()) {
    Handle<ObjectTemplate> raw_template = MakeKeyCodeTemplate();
    keycode_template = Persistent<ObjectTemplate>::New(raw_template);
  }

  Persistent<Object> kc = Persistent<Object>::New(
          keycode_template->NewInstance());

  kc.MakeWeak(nullptr, CleanupKeycode);

  ASSERT(kc->InternalFieldCount() == 1);
  kc->SetInternalField(0, External::New(this));
  return kc;
}

namespace {
const size_t max_code = %(max_code)d;
const char * keycode_arr[%(arr_size)d] = {
%(codes)s
};
}

KeyCode* CursesToKeycode(const wint_t &wch, bool is_keypad) {
  // The returned pointers are "owned" by V8; the way they'll get deleted
  // later on is by CleanupKeycode, which will be invoked when the containing
  // V8 object is garbage collected.
  if (is_keypad) {
    size_t offset = static_cast<size_t>(wch);
    ASSERT(offset <= max_code);
    const char *name = keycode_arr[offset];
    ASSERT(name != nullptr);
    return new KeyCode(wch, name);
  } else {
    return new KeyCode(wch);
  }
}

Handle<Object> GetKeycodeMap() {
  HandleScope scope;
  Handle<Object> arr = Object::New();
  %(keycode_map_code)s
  return scope.Close(arr);
}
}
"""

key_regex = re.compile(r'^([_a-z0-9]+)\s+[a-zA-Z0-9]+\s+str\s+[a-zA-Z0-9;@%&*#!]+\s+([_A-Z()0-9]+)\s+([-0-9]+)\s+[-A-Z*]+\s+(.*)$')
octal_regex = re.compile(r'^0[0-9]+$')
define_regex = re.compile(r'^#define (KEY_[A-Z_]+)\s+([0-9]+)\s+/\*(.*)\*/\s*$')


def already_in_values(values, code):
    return any(c == code for _, _, c, _ in values)

def parse_capabilities(caps_path, values):
    with open(caps_path) as in_file:
        for line in in_file:
            if not line.startswith('key_'):
                continue
            m = key_regex.match(line)
            if not m:
                print >> sys.stderr, 'failed to parse line %r' % (line,)
                sys.exit(1)
            name, macro, code, description = m.groups()
            if name.startswith('key_'):
                name = name.upper()
            if macro.startswith('key_'):
                macro = macro.upper()
            if code == '-':
                code = 0
            elif octal_regex.match(code):
                code = int(code, 8)
            else:
                print >> sys.stderr, 'failed to parse octal code %r' % (code,)
            if not already_in_values(values, code):
                values.append((name, macro, code, description))
    return values

def parse_curses(curses_path, values):
    with open(curses_path) as in_file:
        for line in in_file:
            m = define_regex.match(line)
            if not m:
                continue
            macro, code, description = m.groups()
            description = description.strip()
            code = int(code, 8)
            if not already_in_values(values, code):
                values.append((macro, macro, code, description))
    return values

if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-o', '--output-prefix', default='src/keycode', help='output prefix')
    parser.add_option('--caps-file', default='third_party/Caps', help='path to the capabilities file')
    parser.add_option('--curses-header', default='/usr/include/ncurses.h', help='path to the ncurses file')
    opts, args = parser.parse_args()

    values = []
    if opts.caps_file:
        parse_capabilities(opts.caps_file, values)
    if opts.curses_header:
        parse_curses(opts.curses_header, values)

    # OK, we were able to parse the file; generate the C++ files

    def comparator(a, b):
        ak = a[2]
        bk = b[2]
        if ak == 0 and bk == 0:
            return cmp(a[0], b[0])
        elif ak == 0:
            return 1
        elif bk == 0:
            return -1
        else:
            return cmp(a[2], b[2])

    values.sort(comparator)
    value_map = dict((code, (name, description)) for name, _, code, description in values)
    max_code = max(value_map.iterkeys())

    current_year = datetime.date.today().year
    h_name = opts.output_prefix + '.h'
    cc_name = opts.output_prefix + '.cc'

    printable = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ '
    code_arr = []
    map_arr = []
    comment_width = 30
    for code in xrange(max_code + 1):
        if code and code in value_map:
            name, description = value_map[code]
            val = '      "%s",' % (name,)
            val += ' ' * (comment_width - len(val))
            val += '// %s' % (description,)
            code_arr.append(val)
            map_arr.append('arr->Set(String::New("%s"), Integer::New(%d), v8::ReadOnly);' % (name, code));
        elif code < 128:
            name = chr(code)
            if name in printable:
                name = name.replace('\\', '\\\\')
                name = name.replace('"', '\\"')
                code_arr.append('      "%s",' % (name,))
                map_arr.append('arr->Set(String::New("%s"), Integer::New(%d), v8::ReadOnly);' % (name, code));
            else:
                code_arr.append('      "\\x%02x",' % (code,))
        else:
            val = '      nullptr,'
            val += ' ' * (comment_width - len(val))
            val += '// keycode %d' % (code,)
            code_arr.append(val)

    with open(h_name, 'w') as h_file:
        h_file.write(h_template.lstrip() % {'current_year': current_year})

    with open(cc_name, 'w') as cc_file:
        cc_file.write(cc_template.lstrip() % ({'arr_size': max_code + 1,
                                               'current_year': current_year,
                                               'codes': '\n'.join(code_arr),
                                               'h_name': os.path.basename(h_name),
                                               'keycode_map_code': '\n  '.join(map_arr),
                                               'max_code': max_code}))
