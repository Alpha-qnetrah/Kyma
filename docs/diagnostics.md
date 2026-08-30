# Diagnostics

All frontend failures are represented by `Diagnostic { message, SourceLocation, warning }`. Lexer and parser failures stop at the first malformed construct with line and column. The analyzer can report multiple independent type/name errors in one pass; warnings do not execute as errors, but are printed by the CLI. Runtime errors use the same user-facing format and preserve the invariant that invalid programs never begin execution.

Future diagnostics will add source spans, notes, related locations, and a call-stack section without changing the public pipeline API.
