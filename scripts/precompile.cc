// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
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
  // first, read the file contents
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

  // now pre-compile the data
  v8::V8::Initialize();
  v8::ScriptData *script_data = v8::ScriptData::PreCompile(contents.c_str(),
                                                           contents.length());
  assert(script_data->HasError() == false);
  std::string marshalled(script_data->Data(), script_data->Length());

  size_t offset = 0;
  while (offset < marshalled.length()) {
    size_t written = fwrite(marshalled.c_str(), 1,
                            marshalled.length() - offset, stdout);
    assert(written != 0);
    offset += written;
  }
  return 0;
}
