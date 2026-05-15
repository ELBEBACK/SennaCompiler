.PHONY: all build install dot clean uninstall test

BUILD_DIR := build
PREFIX    := $(HOME)/.local

all: build

build:
	cmake -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) -j$(shell nproc)

install: build
	cmake --install $(BUILD_DIR) --prefix $(PREFIX)

dot:
	./scripts/dot2png.sh

clean:
	rm -rf $(BUILD_DIR) output/

uninstall:
	rm -rf $(PREFIX)/bin/senna

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure
