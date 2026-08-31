# Architectural decisions

1. **C++20, no runtime dependency.** The implementation uses standard-library variants, maps, smart pointers, and visitors. This keeps the language machinery inspectable and portable.
2. **Braces and semicolons are semantic.** The lexer discards whitespace; the parser requires explicit statement termination.
3. **Validated phase boundaries.** Source inspection follows syntax → HIR → verified MIR → validated bytecode. Execution never accepts a failed frontend result.
4. **Lexical closures.** Functions capture environments, not source text or host callbacks.
5. **Closed objects.** Object shapes are not arbitrary property bags; dynamic behavior must be explicit in a future Map/Record type.
6. **Diagnostics are data.** Source locations travel with tokens and AST nodes, and the CLI formats diagnostics consistently.
7. **Staged VM migration.** The tree walker remains the compatibility reference while constructs move through HIR and MIR into the register VM. Unsupported lowering is explicit and diagnostic, never a silent fallback inside inspection commands.
