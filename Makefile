BUILD_DIR ?= build
GENERATOR ?= Unix Makefiles
PREFIX ?= $(HOME)/.local

.PHONY: all configure build release test asan install run clean
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

asan:
	cmake -S . -B build-asan -G "$(GENERATOR)" -DCMAKE_BUILD_TYPE=Debug -DKYMA_ENABLE_SANITIZERS=ON
	cmake --build build-asan
	ctest --test-dir build-asan --output-on-failure

install: build
	cmake --install $(BUILD_DIR) --prefix "$(PREFIX)"
	@echo "Installed kyma to $(PREFIX)/bin/kyma"
	@echo "Ensure $(PREFIX)/bin is on PATH."

run: build
	@test -n "$(FILE)" || (echo "usage: make run FILE=examples/hello.kyma"; exit 2)
	./$(BUILD_DIR)/kyma $(FILE)

clean:
	rm -rf build build-release build-asan
