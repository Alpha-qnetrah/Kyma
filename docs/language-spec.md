# Kyma language specification (v0.1)

## Lexical rules

UTF-8 source is scanned as ordinary characters; whitespace and indentation have no semantic meaning. `//` and `/* ... */` comments are ignored. Blocks use `{}` and ordinary statements end with `;`. Identifiers begin with a letter or `_`. String literals use double quotes and character literals use single quotes.

## Bindings and types

`let` creates a mutable, type-locked binding. `set` creates an immutable binding. A type is inferred from an initializer unless written after `:`. `any` is the explicit dynamic escape hatch. The built-ins are `int`, `float`, `num`, `str`, `char`, `bool`, `null`, `void`, and `any`; `int` and `float` are distinct and both are compatible with `num`. Types are non-nullable by default; `T?` is sugar for `T | null`.

```kyma
let count: int = 1;
set title = "Kyma";
let maybe: str? = null;
```

Objects are closed: an assignment to a field not present in the object/class shape is rejected by the runtime. Arrays use `[a, b]`, zero-based integer indexing, and are mutable reference values; `len`, `push`, and `pop` are standard-library operations. `null` is a value and `void` describes absence of a function result.

## Functions and control flow

Parameters always have types and calls always use parentheses. Return annotations are optional for safely inferred functions and mandatory contracts when present. `if` requires a parenthesized condition and braced branches. `while` and `loop (init; condition; increment)` are statements. `break` and `continue` may name a loop label. `match` currently accepts literal and `_` arms; arms use `=>` and terminate with `;`. `try { ... } catch (message) { ... }` catches language/runtime errors as a string binding; `error(message)` raises one. `if` is expression-capable; loops are not.

## Objects and classes

Class inheritance is single-parent. Constructors are named `init`, construction uses `new`, and instance access requires explicit `self`. `super.member` resolves a parent member. Class methods may be `public`, `private`, `protected`, `static`, `override`, `final`, or `abstract` modifiers; the parser preserves modifiers for the semantic checker roadmap. `abstract class` cannot be instantiated. `intf` declares a no-state interface shape. Generic syntax, traits, and complete access/conformance checking are planned extensions, not silently treated as `any`.
