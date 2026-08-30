# Automatic garbage collection

Kyma v0.1 adds a small tracing heap for runtime objects. `Heap` owns `Object` storage; `Value` object edges are non-owning pointers. At safe interpreter statement boundaries, `maybeCollect` checks an allocation threshold and runs mark-and-sweep when needed. `collectGarbage()` is also exposed for deterministic tests and diagnostics.

Roots are the global environment and the active lexical environment. The marker follows object fields, function receivers and closures, class static fields, and class method closures. Because object edges are non-owning, unreachable reference cycles are reclaimed instead of being leaked by reference counting. Collection never runs in the middle of an expression.

The design deliberately separates allocation (`Heap::allocate`) from traversal and policy (`collect`/`maybeCollect`). A future generational or concurrent collector can replace the policy while preserving runtime `Value` and interpreter APIs. `gcStats()` reports live objects and collection count; it is diagnostic information, not language semantics.
