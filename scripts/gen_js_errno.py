import datetime
import errno

template = """
// Copyright %(year)d, Evan Klitzke <evan@eklitzke.org>
//
// Definitions for various errorcodes. This file is autogenerated by
// scripts/gen_js_errno.py.

#include "./js_errno.h"

#include <errno.h>
#include <v8.h>

#include "./module.h"

namespace e {
namespace js_errno {
bool Build(v8::Handle<v8::Object> obj) {
  v8::HandleScope scope;
%(values)s

  v8::Local<v8::Array> arr = v8::Array::New(%(max_errno)d);
%(array)s

  v8::Local<v8::String> key = v8::String::New("errorcode");
  obj->Set(key, arr);

  return true;
}
}
}
"""

if __name__ == '__main__':
    values = []
    for name in sorted(errno.errorcode.itervalues()):
        values.append('  AddInteger(obj, "%(e)s", %(e)s);' % {'e': name})
    values = '\n'.join(values)

    array = []
    for k, v in errno.errorcode.iteritems():
        array.append('  arr->Set(%d, v8::String::New("%s"));' % (k, v))
    array = '\n'.join(array)

    print template.strip() % {'array': array,
                              'max_errno': max(errno.errorcode.iterkeys()) + 1,
                              'values': values,
                              'year': datetime.datetime.today().year}
