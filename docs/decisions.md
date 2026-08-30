# Architectural decisions

1. **C++20, no runtime dependency.** The implementation uses standard-library variants, maps, smart pointers, and visitors. This keeps the language machinery inspectable and portable.
2. **Braces and semicolons are semantic.** The lexer discards whitespace; the parser requires explicit statement termination.
3. **AST before execution.** The interpreter receives validated AST nodes only, preserving the later bytecode compiler seam.
4. **Lexical closures.** Functions capture environments, not source text or host callbacks.
5. **Closed objects.** Object shapes are not arbitrary property bags; dynamic behavior must be explicit in a future Map/Record type.
6. **Diagnostics are data.** Source locations travel with tokens and AST nodes, and the CLI formats diagnostics consistently.
7. **No premature VM.** The tree walker is the reference semantics. The tracing heap is isolated behind an allocation/collection seam so a bytecode VM can reuse it.
