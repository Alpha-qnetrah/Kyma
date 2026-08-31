# Automatic garbage collection

`ManagedHeap` is the public v0.2 allocation boundary. It owns objects and arrays; `RuntimeValue` stores non-owning references to those nodes. Collection occurs only at interpreter allocation safepoints or through `collectGarbage()`.

Roots include globals, module environments, active lexical environments, closures, bound receivers, class state, and module namespaces. Marking uses an iterative worklist, so deep object/array graphs cannot overflow the C++ stack. Because managed object edges are non-owning, unreachable cycles are reclaimed.

The growth policy uses live heap size rather than lifetime allocation count. C++ `HeapStats` reports live, allocated, reclaimed, collection count, peak live size, and the next threshold. Kyma's existing `gcStats()` string and `collectGarbage()` behavior remain source-compatible.
