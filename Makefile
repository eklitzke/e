SRCFILES := $(shell git ls-files src/)
TESTFILES := $(shell git ls-files tests/)
TARGET := build/out/Default/e
TEST_TARGET := build/out/Default/test
TEMPLATES := $(shell echo scripts/templates/*.html)
BUNDLED_JS = src/bundled_core.cc src/bundled_core.h
KEYCODE_FILES = src/keycode.cc src/keycode.h

all: docs/jsdoc.html

clean:
	rm -rf build/out/ build/src/ docs/ $(BUNDLED_JS) $(KEYCODE_FILES) e test

docs:
	@mkdir -p docs

docs/jsdoc.html: scripts/gen_js_docs.py e docs
	@echo -n "Updating $@..."
	@python scripts/gen_js_docs.py -o $@ $(SRCFILES) $(KEYCODE_FILES)
	@echo " done!"

$(BUNDLED_JS): scripts/gen_bundled_core.py js/core.js js/parser.js
	python $^

$(KEYCODE_FILES): scripts/gen_key_sources.py third_party/Caps
	python $^

build:
	gyp --toplevel-dir=. --depth=src/ --generator-output=build e.gyp

lint:
	python third_party/cpplint.py $(SRCFILES)

$(TARGET): $(SRCFILES) $(BUNDLED_JS) $(KEYCODE_FILES) build
	make -C build e

$(TEST_TARGET): $(SRCFILES) $(TESTFILES) $(BUNDLED_JS) $(KEYCODE_FILES) build
	make -C build test

test: $(TEST_TARGET)

e: $(TARGET)
	@if [ ! -e "$@" ]; then echo -n "Creating ./$@ symlink..."; ln -sf $(TARGET) $@; echo " done!"; fi

test: $(TEST_TARGET)
	@if [ ! -e "$@" ]; then echo -n "Creating ./$@ symlink..."; ln -sf $(TEST_TARGET) $@; echo " done!"; fi


.PHONY: all clean lint test
