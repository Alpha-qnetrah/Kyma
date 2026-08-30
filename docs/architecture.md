# Architecture

Kyma is deliberately split into stages:

`source -> lexer -> tokens -> parser -> AST -> Analyzer -> runtime interpreter`.

The public headers in `include/kyma` expose stable stage boundaries. AST nodes contain source locations and use ownership-friendly `shared_ptr` edges for recursive nodes. Runtime values are a closed `std::variant` today, but the `Value`/`Function`/`Class` boundary is independent of the AST and can later be backed by tagged VM values. Runtime objects are owned by the independent tracing `Heap`.

`Environment` is a lexical parent chain. Bindings carry mutability separately from their value, so `set` freezes a binding rather than an object. Functions capture an environment and invocation creates a fresh call environment. This is the call-stack seam for a future VM.

The CLI never invokes the interpreter if lexing, parsing, or analysis produced an error. Diagnostics carry a source location and stage-specific failures use the same exception type. Runtime failures are also reported without C++ stack traces.

## v0.1 status

Implemented: lexer, comments and literals; expressions; mandatory semicolon statements; lexical blocks; `let`/`set`, inference, `num`, `any`, nullable type syntax; functions and closures; if/else expressions; while and C-style `loop`; break/continue with loop labels; literal/wildcard match; closed object literals; classes, instances, init, self, super, single inheritance, method override syntax, static method lookup, and abstract-class instantiation checks; interfaces are parsed as compile-time declarations; CLI, REPL, tests, and CTest.

The VM, module loader, full access-control checker, structural interface conformance, traits, generic instantiation, arrays/maps, and richer patterns are intentionally reserved for the next milestones. Their syntax and AST seams are documented rather than faked by a transpiler.
