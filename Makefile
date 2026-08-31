BUILD_DIR ?= build
GENERATOR ?= Unix Makefiles
PREFIX ?= $(HOME)/.local
VSCODE ?= /Applications/Visual Studio Code.app/Contents/Resources/app/bin/code
VSCODE_EXTENSION_VERSION := $(shell sed -n 's/.*"version": "\([^"]*\)".*/\1/p' editors/vscode-kyna/package.json | head -1)

.PHONY: all configure build release test architecture-check asan format lint install run vscode-package vscode-install clean
all: build

configure:
	cmake -S . -B $(BUILD_DIR) -G "$(GENERATOR)" -DCMAKE_BUILD_TYPE=Debug

build: configure
	cmake --build $(BUILD_DIR)

release:
	cmake -S . -B build-release -G "$(GENERATOR)" -DCMAKE_BUILD_TYPE=Release
	cmake --build build-release

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

architecture-check:
	python3 build_tools/verify_repository_architecture.py

format:
	@formatter="$${CLANG_FORMAT:-$$(command -v clang-format 2>/dev/null || echo /opt/homebrew/opt/llvm/bin/clang-format)}"; \
	"$$formatter" -i $$(find compiler runtime library sdk tools tests -type f \( -name '*.hpp' -o -name '*.cpp' \) | sort)

lint: build
	@tidy="$${CLANG_TIDY:-$$(command -v clang-tidy 2>/dev/null || echo /opt/homebrew/opt/llvm/bin/clang-tidy)}"; \
	"$$tidy" $$(find compiler runtime library sdk tools tests -type f -name '*.cpp' | sort) -- -std=c++23

asan:
	cmake -S . -B build-asan -G "$(GENERATOR)" -DCMAKE_BUILD_TYPE=Debug -DKYNA_ENABLE_SANITIZERS=ON
	cmake --build build-asan
	ctest --test-dir build-asan --output-on-failure

install: build
	cmake --install $(BUILD_DIR) --prefix "$(PREFIX)"
	@echo "Installed kyna to $(PREFIX)/bin/kyna"
	@echo "Ensure $(PREFIX)/bin is on PATH."

run: build
	@test -n "$(FILE)" || (echo "usage: make run FILE=examples/hello.kyna"; exit 2)
	./$(BUILD_DIR)/bin/kyna $(FILE)

vscode-package:
	sh tools/package-vscode.sh

vscode-install: vscode-package
	"$(VSCODE)" --install-extension editors/vscode-kyna/kyna-language-support-$(VSCODE_EXTENSION_VERSION).vsix --force

clean:
	rm -rf build build-release build-asan
