// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// This is a small program that pre-compiles JavaScript source code. By doing
// this, JavaScript code can be loaded into V8 faster later on.
//
// The JavaScript program is sent to this program on stdin, and then the sender
// closes stdin. After that the program precompiles the data, writes it to
// stdout, and then exits.
//
// You should be able to compile like this:
//     g++ -lv8 precompile.cc -o precompile

#include <v8.h>
#include <cassert>
#include <cstdio>
#include <string>

const size_t buf_size = 2000;

int main(int argc, char **argv) {
  if (argc != 1) {
    fprintf(stderr, "usage: precompile\n");
    return 1;
  }

  // read the file contents from stdin
  char *buf = new char[buf_size];
  std::string contents;
  while (true) {
    size_t bytes_read = fread(buf, 1, buf_size, stdin);
    if (bytes_read == 0) {
      assert(feof(stdin) != 0);
      assert(fclose(stdin) == 0);
      break;
    }
    contents += std::string(buf, bytes_read);
  }
  delete[] buf;

  // pre-compile the data
  v8::V8::Initialize();
  v8::ScriptData *script_data = v8::ScriptData::PreCompile(contents.c_str(),
                                                           contents.length());
  assert(script_data->HasError() == false);
  std::string marshalled(script_data->Data(), script_data->Length());

  // write the pre-compiled data to stdout
  size_t offset = 0;
  while (offset < marshalled.length()) {
    size_t written = fwrite(marshalled.c_str(), 1,
                            marshalled.length() - offset, stdout);
    assert(written != 0);
    offset += written;
  }
  return 0;
}
