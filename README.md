# Kyma

Kyma is a small, strongly typed, brace-delimited programming language. Version 0.2 is a dependency-free C++23 tree-walking implementation with recoverable diagnostics, namespace modules, structural interfaces, class contracts, a tracing heap, a persistent REPL, and editor tooling. Kyma executes its syntax tree directly; it does not transpile source code.

## Build

Requirements: CMake 3.25+, a C++23 compiler, and Make (Ninja is preferred when installed). On the development machine: Apple arm64, Apple Clang 21 (C++23 verified), CMake 4.2.3, Make, Git, Homebrew, and LLDB are installed. Homebrew LLVM 23.1.0 now provides clang-format and clang-tidy. Ninja, Valgrind, and a third-party test framework remain unavailable; the project uses CTest and dependency-free tests. Homebrew LLVM was installed during this setup to provide formatting and linting.

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
kyma run examples/hello.ky
make run FILE=examples/hello.ky
```

Set `PREFIX=/usr/local` when a system-wide installation is appropriate.

Platform archives for Linux, Windows, and macOS are produced from tagged releases. See [distribution.md](docs/distribution.md) for archive contents and installation instructions.

VS Code support is included and installed with:

```sh
make vscode-install
```

It registers `.ky` files, `#` line comments, syntax colors, snippets, declarations and import completions, live diagnostics, and Run/Check editor buttons. `make vscode-package` creates the installable VSIX.

Sanitizers (supported by Apple Clang):

```sh
cmake -S . -B build-asan -G 'Unix Makefiles' -DKYMA_ENABLE_SANITIZERS=ON
cmake --build build-asan && ctest --test-dir build-asan --output-on-failure
```

## Use

```sh
./build/kyma run examples/hello.ky
./build/kyma check examples/hello.ky
./build/kyma tokens examples/hello.ky --format json
./build/kyma ast examples/hello.ky
./build/kyma repl
./build/kyma run examples/fake_api_store.ky
./build/kyma run examples/weather_api.ky
```

`kyma file.ky` remains an alias for `kyma run file.ky`. Add module roots with repeated `--module-path`, choose `--diagnostic-format text|json`, and disable ANSI styling with `--no-color`.

A source file runs only after lexing, parsing, module loading, and semantic analysis succeed. Runtime objects use the automatic tracing `ManagedHeap`; `collectGarbage()` and `gcStats()` remain available. See `docs/` for the language contract, architecture, diagnostics schema, and [v0.1 embedding migration](docs/cpp-api-migration.md).

The standard library includes JSON parsing/stringification, a Fetch-style response object, `console.log`, `filter`, `sort`/`bubbleSort`, `call`, process helpers, filesystem/JSON persistence through `fs`, and an in-memory CRUD store. `examples/fake_api_store.ky` exercises GET, POST, PUT, and DELETE against the real Fake Store API and persists results under `fake-store-output/`; HTTPS requests require the system `curl` executable and honor its standard proxy environment variables.

`examples/weather_api.ky` is a keyless live HTTPS smoke test against Open-Meteo. It validates network transport, nested JSON member access, console output, and writing the response to `weather-output/current.json`.
