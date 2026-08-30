# Modules and evolution

The v0.1 executable is a single compilation unit. Module loading is intentionally a frontend/runtime milestone rather than an implicit filesystem side effect. The planned design is:

- one source file defines a module identity;
- `export` adds a declaration to the module's public table;
- `import path` loads and caches a module, analyzes it once, then exposes only exports;
- resolution uses the importing file's directory first, followed by configured library roots;
- cycles are diagnosed with an import stack;
- module initialization runs once and in dependency order.

The current AST has declaration-level boundaries and the CLI already owns the source-to-pipeline orchestration, so this can be added without changing expression execution or introducing a transpiler.
