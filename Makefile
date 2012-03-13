SRCFILES := $(shell git ls-files src/)
TARGET := gyp.out/out/Default/e
TEMPLATES := $(shell echo scripts/templates/*.html)
BUNDLED_JS = src/bundled_core.cc src/bundled_core.h
KEYCODE_FILES = src/keycode.cc src/keycode.h

all: e docs/jsdoc.html

clean:
	rm -rf gyp.out/out/ docs/ $(BUNDLED_JS) $(KEYCODE_FILES) e

docs:
	mkdir -p docs

docs/jsdoc.html: docs scripts/gen_js_docs.py $(SRCFILES) $(KEYCODE_FILES) $(TEMPLATES)
	python scripts/gen_js_docs.py -o $@ $(SRCFILES) $(KEYCODE_FILES)

$(BUNDLED_JS): scripts/gen_bundled_core.py js/core.js
	python $^

$(KEYCODE_FILES): scripts/gen_key_sources.py third_party/Caps
	python $^

gyp.out:
	gyp --toplevel-dir=. --depth=src/ --generator-output=gyp.out e.gyp

$(TARGET): $(SRCFILES) $(BUNDLED_JS) $(KEYCODE_FILES) gyp.out
	make -C gyp.out

e: $(TARGET)
	@if [ ! -e "e" ]; then echo "Creating ./e symlink..."; ln -s $(TARGET) e; fi


.PHONY: all clean
