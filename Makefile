BUILD_DIR ?= build
GENERATOR ?= Unix Makefiles
PREFIX ?= $(HOME)/.local

.PHONY: all configure build release test asan format lint install run vscode-package vscode-install clean
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

format:
	@formatter="$${CLANG_FORMAT:-$$(command -v clang-format 2>/dev/null || echo /opt/homebrew/opt/llvm/bin/clang-format)}"; \
	"$$formatter" -i $$(find include src tests -type f \( -name '*.hpp' -o -name '*.cpp' \) | sort)

lint: build
	@tidy="$${CLANG_TIDY:-$$(command -v clang-tidy 2>/dev/null || echo /opt/homebrew/opt/llvm/bin/clang-tidy)}"; \
	"$$tidy" $$(find src tests -type f -name '*.cpp' | sort) -- -Iinclude -std=c++23

asan:
	cmake -S . -B build-asan -G "$(GENERATOR)" -DCMAKE_BUILD_TYPE=Debug -DKYMA_ENABLE_SANITIZERS=ON
	cmake --build build-asan
	ctest --test-dir build-asan --output-on-failure

install: build
	cmake --install $(BUILD_DIR) --prefix "$(PREFIX)"
	@echo "Installed kyma to $(PREFIX)/bin/kyma"
	@echo "Ensure $(PREFIX)/bin is on PATH."

run: build
	@test -n "$(FILE)" || (echo "usage: make run FILE=examples/hello.ky"; exit 2)
	./$(BUILD_DIR)/kyma $(FILE)

vscode-package:
	sh tools/package-vscode.sh

vscode-install: vscode-package
	code --install-extension editors/vscode-kyma/kyma-language-support-0.1.0.vsix --force

clean:
	rm -rf build build-release build-asan
