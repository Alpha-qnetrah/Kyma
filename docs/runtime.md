# Runtime model

`Value` stores null, bool, int64, double, string, char, object, function, or class. Numeric representation is preserved (`num` does not erase int versus float). `Object` owns a closed field map and points to its `Class`. `Function` stores a declaration, lexical closure, and optional receiver. `Class` stores methods, static fields, and a parent class.

Calls allocate a child `Environment` containing parameters and (for methods) `self`. The environment chain implements lexical lookup and closures. Assignment searches the chain but refuses immutable cells. `new` allocates an instance, initializes declared fields, and invokes `init`. Objects are owned by the tracing `Heap`; automatic mark-and-sweep runs at safe statement boundaries, and `collectGarbage()` provides an explicit diagnostic hook.

The compatibility interpreter remains tree-walking for constructs not yet migrated. The primary compiler path lowers source through HIR and verified MIR into validated register bytecode. No source-to-source translation, host-language eval, or hidden dynamic fallback is used.
