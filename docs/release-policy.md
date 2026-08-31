# Release policy

A Kyna release is built from a clean, protected tag after Debug, Release, sanitizer, cross-platform, editor, packaging, and architecture checks pass. Release jobs must use the checked-in dependency manifest and must not publish artifacts produced from a developer working tree.

Official artifacts include checksums and a dependency inventory. Production releases require provenance attestation, macOS Developer ID signing and notarization, Windows Authenticode signing, and verification on clean machines. Missing signing credentials block a production release; workflows must not silently publish unsigned installers under an official stable tag.

Marketplace and Open VSX publishing use repository secrets scoped to their respective publishers. Release logs, crash bundles, and diagnostics must redact authorization headers, credentials, and sensitive URL query values.
