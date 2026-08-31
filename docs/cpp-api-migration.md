# C++ embedding migration: v0.1 to v0.2

The broad `#include <kyma/kyma.hpp>` umbrella was removed. Include and link the smallest public domain interface needed by the embedding:

- full checking/running: `kyma/language/language_session.hpp`, link `kyma_language`;
- token inspection: `kyma/lexing/tokenizer.hpp`, link `kyma_syntax`;
- parsing: `kyma/parsing/module_parser.hpp`, link `kyma_syntax`;
- module loading: `kyma/modules/module_loader.hpp`, link `kyma_modules`;
- analysis: `kyma/semantics/module_analyzer.hpp`, link `kyma_semantics`;
- execution: `kyma/execution/tree_walk_interpreter.hpp`, link `kyma_execution`.

Use `tokenize(SourceFile)`, `parseModule(SourceFile, tokens)`, and `analyzeModuleGraph(graph)` result objects instead of catching exceptions for expected source mistakes. `LanguageSession` is the normal high-level replacement for manually sequencing the v0.1 lexer, parser, analyzer, and interpreter.

Host access is injected through `RuntimeCapabilities`. Production adapters are the default; deterministic tests can provide filesystem, process, network, and clock ports. The `kyma_lib` CMake target remains as an interface-only transition alias for v0.1 build files, but it is not the new embedding API.
