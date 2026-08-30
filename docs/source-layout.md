# Source layout

Kyma uses deep modules with narrow interfaces and localized implementations:

- `lexer/` contains scanning; `keywords.cpp` owns keyword classification and `words.cpp` owns token descriptions.
- `types/` owns type representation formatting.
- `parser/` separates declarations/statements from expression precedence.
- `semantic/` separates validation, type checking, modifier behavior, and interface cataloguing.
- `runtime/` separates values, environments, callable dispatch, heap ownership, and collection.
- `stdlib/` contains trusted host adapters (files, processes, console, and HTTP).
- `interpreter/` separates statement execution from expression evaluation.
- `cli/` is the only command-line adapter.
- `tests/<module>/` tests each module through its public interface.
- `editors/vscode-kyma/` is independently packageable editor support for `.ky` files.

The CMake target `kyma_lib` is the shared implementation seam for the CLI, tests, and future compiler/VM frontends.
