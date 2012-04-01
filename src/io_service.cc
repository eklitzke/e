// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#include "./io_service.h"

#include <boost/asio.hpp>

namespace {
boost::asio::io_service io_service_;
}

namespace e {
boost::asio::io_service* GetIOService() {
  return &io_service_;
}
}
