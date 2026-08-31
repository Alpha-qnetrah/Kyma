# Architecture

Kyma v0.2 enforces this dependency direction with separate CMake targets:

`kyma_source -> kyma_syntax -> kyma_modules -> kyma_semantics -> kyma_execution -> kyma_language -> CLI`

`kyma_source` owns source identity, byte spans, line indexing, and diagnostic rendering. `kyma_syntax` owns tokenization, recovery, and syntax construction. `kyma_modules` resolves and caches a canonical import graph. `kyma_semantics` produces a `CheckedProgram`. `kyma_execution` owns runtime values, capability ports, the tree walker, and managed storage. `kyma_language` exposes the orchestration interface used by both the CLI and editor extension.

Expected source mistakes are returned as results. An invalid token stream or syntax tree is never analyzed; a program with semantic errors is never executed. Runtime failures use the same structured diagnostic model.

The old `kyma_lib` target is an interface-only compatibility facade. New embedders should link the narrowest target they need, normally `kyma_language`.
