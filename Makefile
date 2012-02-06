CC := g++ -g

.PHONY: all
all: e

.PHONY: opt
opt: e_opt

buffer.o: buffer.cc
	$(CC) -c $^

state.o: state.cc
	$(CC) -c $^

statusbar.o: statusbar.cc
	$(CC) -c $^

term.o: term.cc
	$(CC) -c $^

e: main.cc buffer.o term.o statusbar.o state.o
	$(CC) $^ -lstdc++ -lncurses -o $@

e_opt: main.cc buffer.o term.o statusbar.o state.o
	$(CC) -Os $^ -lstdc++ -lncurses -o $@
	strip -s $@

PHONY: clean
clean:
	rm -f *.o e
