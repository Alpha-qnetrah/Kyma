# Architecture

Kyna v0.3 enforces this dependency direction with separate CMake targets:

`kyna_source -> kyna_syntax -> kyna_modules -> kyna_semantics -> kyna_execution -> kyna_language -> CLI`

`kyna_source` owns source identity, byte spans, line indexing, and diagnostic rendering. `kyna_syntax` owns tokenization, recovery, and syntax construction. `kyna_modules` resolves and caches a canonical import graph. `kyna_semantics` produces a `CheckedProgram`. `kyna_execution` owns runtime values, capability ports, the tree walker, and managed storage. `kyna_language` exposes the orchestration interface used by both the CLI and editor extension.

Expected source mistakes are returned as results. An invalid token stream or syntax tree is never analyzed; a program with semantic errors is never executed. Runtime failures use the same structured diagnostic model.

The old `kyna_lib` target is an interface-only compatibility facade. New embedders should link the narrowest target they need, normally `kyna_language`.
