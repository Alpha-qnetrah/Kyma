# C++ embedding migration: v0.1 to v0.3

The broad `#include <kyna/kyna.hpp>` umbrella was removed. Include and link the smallest public domain interface needed by the embedding:

- full checking/running: `kyna/language/language_session.hpp`, link `kyna_language`;
- token inspection: `kyna/lexing/tokenizer.hpp`, link `kyna_syntax`;
- parsing: `kyna/parsing/module_parser.hpp`, link `kyna_syntax`;
- module loading: `kyna/modules/module_loader.hpp`, link `kyna_modules`;
- analysis: `kyna/semantics/module_analyzer.hpp`, link `kyna_semantics`;
- execution: `kyna/execution/tree_walk_interpreter.hpp`, link `kyna_execution`.

Use `tokenize(SourceFile)`, `parseModule(SourceFile, tokens)`, and `analyzeModuleGraph(graph)` result objects instead of catching exceptions for expected source mistakes. `LanguageSession` is the normal high-level replacement for manually sequencing the v0.1 lexer, parser, analyzer, and interpreter.

Host access is injected through `RuntimeCapabilities`. Production adapters are the default; deterministic tests can provide filesystem, process, network, and clock ports. The `kyna_lib` CMake target remains as an interface-only transition alias for v0.1 build files, but it is not the new embedding API.
