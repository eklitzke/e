#include <string>
#include <vector>

#include "statusbar.h"

namespace e {

  std::string*
  StatusBar::render(int width)
  {
	std::string *s = new std::string(bufname_);
	return s;
  }

}
