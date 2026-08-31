# Automatic garbage collection

`ManagedHeap` is the public allocation boundary. It owns objects, arrays, VM closures, and VM capture cells; `RuntimeValue` stores typed non-owning references to those nodes. Collection occurs only at interpreter allocation safepoints or through `collectGarbage()`.

Roots include globals, module environments, active lexical environments, VM registers, closures, capture cells, bound receivers, class state, and module namespaces. Marking uses an iterative worklist, so deep object/array/closure graphs cannot overflow the C++ stack. Because managed object edges are non-owning, unreachable cycles—including recursive closures—are reclaimed.

The growth policy uses live heap size rather than lifetime allocation count. Closure allocation is an explicit VM safepoint: active registers, boxed registers, and frame captures form a typed root set before collection. C++ `HeapStats` reports live, allocated, reclaimed, collection count, peak live size, and the next threshold; bytecode execution returns the same statistics for embedding and tests. Kyna's existing `gcStats()` string and `collectGarbage()` behavior remain source-compatible.
