# Testing architecture

Tests are grouped by the module seam they exercise:

- `tests/lexer` checks tokenization and keyword classification;
- `tests/parser` checks AST construction;
- `tests/runtime` checks environments, object lifetime, and collection;
- `tests/stdlib` checks host adapters;
- `tests/test_kyma.cpp` is the end-to-end language suite.

Each test links against the same `kyma_lib` interface used by the CLI. CTest runs all suites in Debug, Release, and sanitizer builds. This keeps tests on the public module interfaces instead of reaching into implementations.
