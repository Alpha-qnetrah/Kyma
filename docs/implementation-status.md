# Implementation status

This is the v0.3 tree-walking boundary, not a claim that deferred VM-era features exist today.

| Area | v0.3 |
|---|---|
| Lexer/parser/source spans | recoverable result interfaces implemented |
| Semantic analysis | names, types, structural interfaces, member/class contracts |
| Tree-walking runtime | implemented |
| Automatic tracing heap / cycle collection | iterative object/array tracing with live growth policy |
| CLI/REPL | run/check/repl/tokens/ast; persistent parser-driven REPL |
| Classes, init, inheritance, self/super | implemented with visibility, override/final, abstract checks |
| Interfaces | explicit class conformance and structural object assignment |
| Arrays / closed objects | implemented baseline |
| try/catch/error and console logging | implemented baseline |
| Namespace imports/exports | canonical graph, cycles, export-only live namespaces |
| VS Code `.kyna` extension | comments, completion, live diagnostics, Run/Check, package asset |
| Files, processes, basic HTTP | injected runtime capability ports |
| HTTPS and HTTP methods | production adapter through system `curl` |
| Streaming and async networking | future |
| Traits/generics | reserved design surface |
| Bytecode VM | future |
| Formatter/linter | future language tools; C++ clang-format/clang-tidy workflow exists |
