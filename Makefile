CC := g++
CFLAGS := -g -Wall
LDFLAGS := -fuse-ld=gold -lncurses -lpthread -lv8
SOURCES=main.cc curses_window.cc window.cc state.cc buffer.cc keycode.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=e

.PHONY: all
all: $(EXECUTABLE)

.PHONY: opt
opt: $(EXECUTABLE)
	strip -s $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: check
check: $(EXECUTABLE)
	cppcheck -q --enable=all .
	python third_party/cpplint.py *.cc *.h

.PHONY: clean
clean:
	rm -f $(EXECUTABLE) *.o *.gch
