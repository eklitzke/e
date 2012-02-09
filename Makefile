CC := g++
CFLAGS := -g -Wall -I../v8/include
#LDFLAGS := -fuse-ld=gold -lncurses
LDFLAGS := -lncurses -lpthread
LINKV8 := -lpthread third_party/libv8.a
SOURCES=main.cc curses_window.cc window.cc state.cc buffer.cc keycode.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=e

.PHONY: all
all: $(EXECUTABLE)

.PHONY: opt
opt: $(EXECUTABLE)
	strip -s $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LINKV8) -o $@

.cc.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: check
check: $(EXECUTABLE)
	cppcheck -q --enable=all .
	python third_party/cpplint.py *.cc *.h

.PHONY: clean
clean:
	rm -f $(EXECUTABLE) *.o *.gch
