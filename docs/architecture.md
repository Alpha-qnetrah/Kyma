# Architecture

Kyna 1.0 is a collection of deep CMake modules with subsystem-owned headers. A caller sees only the interface of the module it links; implementation headers are not shared through a repository-wide include path.

The active compiler direction is:

```text
source → diagnostics/types → lexing → syntax → parsing
       → module resolution → type checking → HIR → MIR → bytecode → VM
       → standard library → embedding session → CLI/editor
```

`kyna_hir` owns stable typed expression, statement, and local IDs. Its syntax-lowering interface resolves local bindings and translates parser-specific operators into a compiler-owned vocabulary. `kyna_mir` owns temporaries, basic blocks, explicit return/goto/branch terminators, verification, and source mappings. `kyna_bytecode` consumes only verified MIR; it no longer depends on syntax nodes.

The bytecode seam consists of a versioned register instruction model, MIR lowering, validation, disassembly, and source mappings. Bytecode v4 records parameter counts, validated call-site argument tables, closure construction, and local/parent capture descriptors. The VM refuses malformed modules before execution and uses an explicit frame stack for calls and recursion rather than the C++ call stack. Nested functions are lifted in HIR and close over heap-owned mutable cells, including transitive and recursive captures. Language constructs not yet lowered continue through the characterized tree-walk engine; `kyna hir`, `kyna mir`, and `kyna bytecode` report a specific `KHIR1201` migration diagnostic instead of silently changing semantics.

Host effects cross injected interfaces. Production filesystem, process, network, PostgreSQL, and clock adapters live in `runtime/kyna_host`; deterministic adapters exercise the same interfaces in tests. Network and database interfaces return typed requests, results, and failure phases rather than opaque error strings. Standard-library modules translate those host-neutral results into managed Kyna values.

Expected source and runtime problems return structured diagnostics. CLI11 owns argument grammar. FTXUI renders diagnostics only on an interactive terminal; pipes, CI, and the VS Code extension receive deterministic text or `kyna.diagnostic/v1` JSON.

See [source-layout.md](source-layout.md) for module ownership and [cpp-api-migration.md](cpp-api-migration.md) for embedding changes.
