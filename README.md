# Kyma

Kyma is a small, strongly typed, brace-delimited programming language. This repository contains the v0.1 tree-walking implementation; it executes Kyma ASTs directly and never transpiles source code.

## Build

Requirements: CMake 3.25+, a C++23 compiler, and Make (Ninja is preferred when installed). On the development machine: Apple arm64, Apple Clang 21 (C++23 verified), CMake 4.2.3, Make, Git, Homebrew, and LLDB are installed. Ninja, clang-format, clang-tidy, Valgrind, and a third-party test framework are not installed; the project therefore uses CTest and a dependency-free assertion test. No package was installed automatically.

```sh
cmake -S . -B build -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The repository also provides a terminal-friendly workflow:

```sh
make                 # configure and build Debug
make test
make install         # installs to ~/.local/bin/kyma
kyma examples/hello.kyma
make run FILE=examples/hello.kyma
```

Set `PREFIX=/usr/local` when a system-wide installation is appropriate.

Sanitizers (supported by Apple Clang):

```sh
cmake -S . -B build-asan -G 'Unix Makefiles' -DKYMA_ENABLE_SANITIZERS=ON
cmake --build build-asan && ctest --test-dir build-asan --output-on-failure
```

## Use

```sh
./build/kyma examples/hello.kyma
./build/kyma --check examples/hello.kyma
./build/kyma --repl
```

A source file runs only after lexing, parsing, and semantic analysis succeed. Runtime objects use the automatic tracing `Heap`; `collectGarbage()` and `gcStats()` are available for diagnostics. See `docs/` for the language contract and implementation boundaries.
