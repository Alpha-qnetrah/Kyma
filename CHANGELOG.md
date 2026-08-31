# Changelog

All notable user-visible changes are recorded here. Kyna follows semantic versioning after 1.0.

## 1.0.0 — unreleased

- Renamed the language, CLI, extension, repository metadata, and canonical source extension to Kyna and `.kyna`.
- Introduced subsystem-owned CMake modules and an architecture verification gate.
- Added a versioned register-bytecode model, validator, disassembler, and initial virtual machine.
- Replaced shell-based network execution with linked libcurl HTTP/HTTPS capabilities and structured network diagnostics.
- Migrated command parsing to CLI11 and interactive diagnostic presentation to FTXUI.
- Added `kyna.diagnostic/v1` JSON diagnostics, lints for fallible operations and missing fetch timeouts, managed-heap temporary roots, and Debug/Release/sanitizer coverage.
- Packaged the VS Code extension as `kyna-lang.kyna-language-support` 1.0.0 with only `.kyna` registration and purple primary assets.

This release remains unreleased until the gates in [ROADMAP.md](ROADMAP.md) are complete.
