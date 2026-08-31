# Testing architecture

Tests are grouped by the module seam they exercise:

- `tests/lexer` checks tokenization and keyword classification;
- `tests/parser` checks AST construction;
- `tests/runtime` checks environments, object lifetime, and collection;
- `tests/stdlib` checks host adapters;
- `tests/v03` checks result interfaces, byte spans, recovery, modules, structural interfaces, access control, and source overlays;
- `tests/test_kyna.cpp` is the end-to-end language suite.

Tests link public domain targets through the compatibility aggregate; no test reaches into a private helper. The release and sanitizer configurations run the same CTest inventory.
