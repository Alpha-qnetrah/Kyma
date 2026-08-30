# Type system

The analyzer uses nominal primitive names with explicit nullable and union components. It checks declarations before execution. Compatibility is directional: `int` and `float` fit `num`; `any` bypasses static compatibility only where explicitly written; a value fits a union if it fits one arm; `null` fits nullable types. Unrelated inferred types do not widen to `any`.

Binding mutability is independent of type. The analyzer records `let`/`set` mutability and rejects assignments to `set`, while runtime cells repeat the check as a safety boundary. Members are similarly separate from bindings, allowing a mutable field through an immutable object reference.

Function parameter types and explicit return contracts are checked. An omitted return annotation is currently inferred dynamically as a function result type at call sites; explicit annotations remain strict. `void` and `null` are distinct. The next checker milestone will add control-flow return-path analysis, member typing, structural interfaces, trait obligations, generic constraints, and complete override/final/access checks.
