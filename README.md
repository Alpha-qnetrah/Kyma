# Kyma

Kyma is a small, strongly typed, brace-delimited programming language. This repository contains the v0.1 tree-walking implementation; it executes Kyma ASTs directly and never transpiles source code.

## Build

Requirements: CMake 3.25+, a C++23 compiler, and Make (Ninja is preferred when installed). On the development machine: Apple arm64, Apple Clang 21 (C++23 verified), CMake 4.2.3, Make, Git, Homebrew, and LLDB are installed. Homebrew LLVM 23.1.0 now provides clang-format and clang-tidy. Ninja, Valgrind, and a third-party test framework remain unavailable; the project uses CTest and dependency-free tests.

```sh
cmake -S . -B build -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The repository also provides a terminal-friendly workflow:

```sh
make                 # configure and build Debug
make test
make format
make install         # installs to ~/.local/bin/kyma
kyma examples/hello.ky
make run FILE=examples/hello.ky
```

Set `PREFIX=/usr/local` when a system-wide installation is appropriate.

VS Code support is included and installed with:

```sh
make vscode-install
```

It registers `.ky` files with syntax colors, snippets, bracket/comment behavior, and Run/Check commands. `make vscode-package` creates the installable VSIX.

Sanitizers (supported by Apple Clang):

```sh
cmake -S . -B build-asan -G 'Unix Makefiles' -DKYMA_ENABLE_SANITIZERS=ON
cmake --build build-asan && ctest --test-dir build-asan --output-on-failure
```

## Use

```sh
./build/kyma examples/hello.ky
./build/kyma --check examples/hello.ky
./build/kyma --repl
```

A source file runs only after lexing, parsing, and semantic analysis succeed. Runtime objects use the automatic tracing `Heap`; `collectGarbage()` and `gcStats()` are available for diagnostics. See `docs/` for the language contract and implementation boundaries.
