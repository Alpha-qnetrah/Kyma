# Kyna

<p align="center">
  <img src="editors/vscode-kyna/assets/kyna-k.svg" width="112" alt="Kyna logo">
</p>

<p align="center"><strong>A small, typed language for readable scripts and language-tool experiments.</strong></p>

<p align="center">
  <a href="https://github.com/Alpha-qnetrah/Kyna/releases">Releases</a> ·
  <a href="docs/language-spec.md">Language specification</a> ·
  <a href="editors/vscode-kyna/README.md">VS Code support</a>
</p>

Kyna is a brace-delimited, strongly typed programming language implemented in dependency-free C++23. The v0.3 runtime is a tree-walking interpreter with structured diagnostics, recoverable parsing, modules, structural interfaces, class contracts, a tracing heap, host capabilities, and a persistent REPL.

## Highlights

- Clear `let` and `set` bindings, functions, classes, interfaces, modules, and `try`/`catch`.
- Python-style `#` comments and JavaScript-style `console.log` output.
- JSON, HTTP(S), filesystem, process, collection, and in-memory CRUD helpers.
- Compiler diagnostics with stable codes, source spans, text/JSON output, and best-practice warnings.
- A VS Code extension with syntax highlighting, snippets, declarations, import completion, live checking, and Run/Check buttons.
- Non-moving managed heap with explicit collection and runtime statistics.

## Quick start

Requirements: CMake 3.25 or newer, a C++23 compiler, and a POSIX shell (Windows builds run in GitHub Actions with MSVC).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Run a program:

```sh
./build/kyna run examples/hello.kyna --no-color
./build/kyna check examples/hello.kyna --no-color
./build/kyna repl
```

The CLI also supports `tokens`, `ast`, repeated `--module-path`, `--diagnostic-format text|json`, and `kyna file.kyna` as a `run` alias. See [distribution.md](docs/distribution.md) for Linux, Windows, and macOS archives.

## A small Kyna program

```kyna
func greet(name: str): str {
    return "Hello " + name;
}

let age = 21;
set category = if (age >= 18) {
    "adult"
} else {
    "minor"
};

console.log(greet("Kyna"), category);
```

Modules use explicit imports and exports:

```kyna
import "./math.kyna" as math;

console.log(math.add(2, 3));
```

Read the complete grammar and semantics in the [language specification](docs/language-spec.md), [module guide](docs/modules.md), and [type-system guide](docs/type-system.md).

## Standard library examples

```kyna
set response = fetch("https://api.open-meteo.com/v1/forecast?latitude=30.04&longitude=31.24&current=temperature_2m");
set weather = response.json();
console.log("temperature", weather.current.temperature_2m);

func adult(user: any): bool {
    return user.age >= 18;
}

set adults = filter(users, adult);
set ordered = sort(adults);
fs.createDirectory("output");
fs.writeJson("output/users.json", ordered);
```

Runnable end-to-end examples live in [`examples/`](examples/), including a keyless Open-Meteo HTTPS smoke test and a Fake Store CRUD example. Network calls use the injected runtime capability and the system `curl`; they never embed credentials.

## VS Code

Build and install the extension locally:

```sh
make vscode-package
code --install-extension editors/vscode-kyna/kyna-language-support-0.3.0.vsix --force
```

The extension registers the canonical `.kyna` extension and the legacy `.ky` compatibility extension, plus `#` comments, completions for Kyna words/declarations/imports, live diagnostics, and editor Run/Check commands. Its primary icon is purple; the black-and-white branding assets remain available in [`editors/vscode-kyna/assets/`](editors/vscode-kyna/assets/).

## Documentation

| Topic | Guide |
| --- | --- |
| Language syntax and semantics | [language-spec.md](docs/language-spec.md), [grammar.md](docs/grammar.md) |
| Architecture and source layout | [architecture.md](docs/architecture.md), [source-layout.md](docs/source-layout.md) |
| Runtime, modules, and memory | [runtime.md](docs/runtime.md), [modules.md](docs/modules.md), [garbage-collection.md](docs/garbage-collection.md) |
| Diagnostics and tooling | [diagnostics.md](docs/diagnostics.md), [distribution.md](docs/distribution.md) |
| Safe installation | [installation-security.md](docs/installation-security.md) |
| C++ embedding migration | [cpp-api-migration.md](docs/cpp-api-migration.md) |

## Project layout

The public C++ layers follow `source → syntax → modules → semantics → execution → language → CLI`. Domain-specific headers are under [`include/kyna/`](include/kyna/) and implementations are split by lexer, parser, semantic pass, runtime, memory, standard-library, and CLI responsibility.

## Contributing and security

Please read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request. To report a security vulnerability privately, follow [SECURITY.md](SECURITY.md) rather than opening a public issue. The macOS “Move to Bin” warning is covered in the [safe installation guide](docs/installation-security.md).

## License

Kyna is released under the [MIT License](LICENSE).
