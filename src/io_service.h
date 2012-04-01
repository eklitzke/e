// -*- C++ -*-
// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

#ifndef SRC_IO_SERVICE_H_
#define SRC_IO_SERVICE_H_

#include <boost/asio.hpp>

namespace e {
boost::asio::io_service* GetIOService();
}

#endif  // SRC_IO_SERVICE_H_
