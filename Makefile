SRCFILES := $(shell echo src/*.cc src/*.h)

.PHONY: all
all: gyp_out

docs/jsdoc.html: scripts/gen_js_docs.py $(SRCFILES)
	echo $^
	python scripts/gen_js_docs.py -o $@

src/bundled_core.cc src/bundled_core.h: js/core.js scripts/gen_bundled_core.py
	python scripts/gen_bundled_core.py -f $<

.PHONY: gyp_out
gyp_out: src/bundled_core.cc src/bundled_core.h
	make -C gyp.out
