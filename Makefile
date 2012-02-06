CC := g++
CFLAGS := -g -c -Wall
#LDFLAGS := -fuse-ld=gold -lncurses
LDFLAGS := -lncurses
SOURCES=buffer.cc statusbar.cc state.cc curses_window.cc main.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=e

.PHONY: all
all: $(SOURCES) $(EXECUTABLE)

.PHONY: opt
opt: $(EXECUTABLE)
	strip -s $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -g $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
