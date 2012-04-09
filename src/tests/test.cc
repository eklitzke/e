// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#define BOOST_TEST_MODULE LineTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../line.h"
#include "../logging.h"

class GlobalConfig {
 public:
  GlobalConfig() { e::InitLogging("test.log"); }
};

BOOST_GLOBAL_FIXTURE(GlobalConfig)

void CheckString(const e::Line &line, const std::string &expectation) {
  BOOST_CHECK(line.ToString() == expectation);
  BOOST_CHECK(line.Size() == expectation.size());
}

BOOST_AUTO_TEST_CASE(insert_char_test) {
  e::Line l;
  CheckString(l, "");

  l.InsertChar(l.Size(), 'f');
  CheckString(l, "f");

  l.InsertChar(l.Size(), 'o');
  CheckString(l, "fo");

  l.InsertChar(l.Size(), 'o');
  CheckString(l, "foo");
}

BOOST_AUTO_TEST_CASE(erase_test) {
  e::Line l("foobar");
  CheckString(l, "foobar");

  l.Erase(3, 3);
  CheckString(l, "foo");

  l.Erase(0, 0);
  CheckString(l, "foo");

  l.Erase(0, 3);
  CheckString(l, "");
}

BOOST_AUTO_TEST_CASE(replace_test) {
  e::Line l("foobar");
  BOOST_CHECK(l.ToString() == "foobar");
  BOOST_CHECK(l.Size() == 6);

  l.Replace("lol");
  BOOST_CHECK(l.ToString() == "lol");
  BOOST_CHECK(l.Size() == 3);
}

BOOST_AUTO_TEST_CASE(chop_test) {
  e::Line l("foobar");
  CheckString(l, "foobar");

  l.Chop(3);
  CheckString(l, "foo");

  l.Chop(0);
  CheckString(l, "");
}

BOOST_AUTO_TEST_CASE(append_test) {
  e::Line l;
  CheckString(l, "");

  uint16_t foo_chars[3] = {'f', 'o', 'o'};
  l.Append(foo_chars, 3);
  CheckString(l, "foo");

  uint16_t bar_chars[3] = {'b', 'a', 'r'};
  l.Append(bar_chars, 3);
  CheckString(l, "foobar");
}
