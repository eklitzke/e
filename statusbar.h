// -*- C++ -*-

namespace e {

  enum Mode {
	COMMAND,
	INSERT
  };

  class StatusBar {
  private:
	bool is_modified_; // is the buffer modified?
	Mode mode_; // the buffer mode

	std::string bufname_;

	int line_;
	int col_;
	int pct_;

	std::string mm_name_; // major mode name
	time_t time_;
  public:
	std::string* render(int);
  };

}
