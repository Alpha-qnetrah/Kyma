# Security policy

## Supported versions

Security fixes are applied to the latest `main` branch and the latest tagged release. Older tags may not receive fixes.

## Reporting a vulnerability

Please do not open a public issue for a suspected vulnerability. Use the repository’s **Security → Report a vulnerability** flow to send a private GitHub Security Advisory to the maintainers. Include:

- the affected version or commit;
- a clear description and reproduction steps;
- impact and any required permissions; and
- a suggested mitigation, if known.

We will acknowledge reports as soon as practical, investigate privately, and coordinate disclosure after a fix is available. Do not include passwords, access tokens, personal data, or live API credentials in a report.

## Scope and safety notes

Kyna executes source code and intentionally exposes opt-in filesystem, process, clock, and network capabilities. Review scripts before running them. The standard library does not provide a sandbox; capability injection is the boundary used by production and deterministic test hosts.
