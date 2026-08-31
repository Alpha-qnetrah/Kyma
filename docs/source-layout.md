# Source layout

Kyna uses deep modules with narrow interfaces and localized implementations:

- `source/` owns source files, identity, byte offsets, and line indexing.
- `diagnostics/` owns versioned text and JSON presentation.
- `lexing/` separates keyword classification, token descriptions, and recoverable tokenization.
- `parsing/` separates declarations/statements, expression precedence, and the module result interface.
- `modules/` owns canonical resolution, graph construction, caching, and cycle reports.
- `semantics/` owns type and class/interface contracts and checked programs.
- `runtime/`, `memory/`, and `execution/` separate values, environments, heap policy, host capabilities, evaluation, and module initialization.
- `stdlib/standard_library_catalog.cpp` binds built-ins to injected capability ports.
- `language/` contains the high-level `LanguageSession` orchestration boundary.
- `cli/` is the only command-line adapter.
- `tests/<module>/` tests each module through its public interface.
- `editors/vscode-kyna/` is independently packageable editor support for `.kyna` files.

Files are named for one language responsibility. Generic implementation names such as `values.cpp`, `functions.cpp`, `behavior.cpp`, `validation.cpp`, and `words.cpp` are no longer used.
