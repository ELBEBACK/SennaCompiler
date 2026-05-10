.PHONY: all build install dot clean uninstall

BUILD_DIR := build
PREFIX    := $(HOME)/.local

all: build

build:
	cmake -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) -j$(shell nproc)

install: build
	cmake --install $(BUILD_DIR) --prefix $(PREFIX)

dot:
	./scripts/dot_to_png.sh

clean:
	rm -rf $(BUILD_DIR) output/

uninstall:
	rm -rf $(PREFIX)/bin/senna