SRCFILES := $(shell git ls-files src/)
TARGET := $(shell readlink ./e)
BUNDLED_JS = src/bundled_core.cc src/bundled_core.h
KEYCODE_FILES = src/keycode.cc src/keycode.h

all: docs/jsdoc.html $(TARGET)

clean:
	rm -rf gyp.out/out docs/jsdoc.html $(BUNDLED_JS) $(KEYCODE_FILES)

docs/jsdoc.html: scripts/gen_js_docs.py $(SRCFILES) $(KEYCODE_FILES)
	python scripts/gen_js_docs.py -o $@ $(SRCFILES) $(KEYCODE_FILES)

$(BUNDLED_JS): scripts/gen_bundled_core.py js/core.js
	python $^

$(KEYCODE_FILES): scripts/gen_key_sources.py third_party/Caps
	python $^

gyp.out:
	gyp --toplevel-dir=. --depth=src/ --generator-output=gyp.out e.gyp

$(TARGET): $(BUNDLED_JS) $(KEYCODE_FILES) gyp.out
	make -C gyp.out

.PHONY: all clean
