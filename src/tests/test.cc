// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#define BOOST_TEST_MODULE LineTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <glog/logging.h>
#include "../line.h"

class GlobalConfig {
 public:
  GlobalConfig() { google::InitGoogleLogging("main"); }
};

BOOST_GLOBAL_FIXTURE(GlobalConfig)

BOOST_AUTO_TEST_CASE(my_test) {
  e::Line l;

  BOOST_CHECK(l.ToString() == "");

  l.InsertChar(l.Size(), 'f');
  BOOST_CHECK(l.ToString() == "f");

  l.InsertChar(l.Size(), 'o');
  BOOST_CHECK(l.ToString() == "fo");

  l.InsertChar(l.Size(), 'o');
  BOOST_CHECK(l.ToString() == "foo");

  l.Erase(0);
  BOOST_CHECK(l.ToString() == "oo");

  l.Erase(0, 2);
  BOOST_CHECK(l.ToString() == "");
}
