# Diagnostics

Diagnostics carry severity, a stable code, message, byte-accurate primary `SourceSpan`, secondary labels, notes, help, and runtime-frame storage. The lexer and parser recover at statement and declaration boundaries and can report multiple independent mistakes. Invalid trees are not analyzed or executed.

Text output prints the source line and an underline. `--diagnostic-format json` emits schema version 1 with the same code, severity, file, and range data; this is the contract used by the VS Code extension. Errors produce exit status `1`, warnings do not prevent checking or execution, and CLI usage or module/file resolution failures produce status `2`.

The semantic best-practice pass currently reports:

- `K2601`: a fallible network, filesystem, or JSON operation is outside a protecting `try` block.
- `K2602`: an empty `catch` block silently hides a failure.
- `K2603`: a literal network URL uses unencrypted `http://`.
- `K2604`: a host shell command requires trusted input.
- `K2605`: the result of a value-producing operation is ignored.

These diagnostics are emitted by the compiler rather than reconstructed by editor tooling, so CLI text, CLI JSON, and VS Code agree on the rule and source span.
