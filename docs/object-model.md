# Object model roadmap

Kyma uses one nominal class parent. `intf` is intended to remain structural: a class or closed object satisfies an interface when its required fields and methods are compatible; no interface state or implementation is copied. `trait` will be behaviour composition with explicit conflict resolution and no class identity. A class may implement many interfaces and compose many traits, but may extend only one class.

Generic declarations will be represented as type parameters with optional upper bounds, for example `class Box<T>` and `func first<T>(items: List<T>): T`. Instantiation specializes types in the analyzer; runtime class/function objects remain monomorphic and carry no source-language type erasure surprises. Generic constraints are checked before interpretation.

The current AST stores class modifiers, method modifiers, and interface members so these rules have a stable place to be added. v0.1 intentionally does not pretend that unimplemented traits or generics are `any`.
