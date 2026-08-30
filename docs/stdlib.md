# Standard library

The v0.1 runtime exposes native functions through the global environment rather than special-casing them in the parser:

- `print(value, ...)`, `typeOf(value)`
- `len`, `push`, `pop`, and `keys` for arrays, strings, and closed objects
- `readFile(path)` and `writeFile(path, content)`
- `processRun(command)`, `processEnv(name)`, and `sleep(milliseconds)`
- `httpGet(url)` for basic plaintext HTTP/1.1 GET requests
- `collectGarbage()` and `gcStats()` for heap diagnostics

HTTP deliberately supports only `http://` in this initial standard library; HTTPS/TLS, streaming, async I/O, and cancellation belong in a future capability-safe networking module. Process execution uses the host shell and should be treated as an explicitly trusted operation. Future standard-library modules will use the planned module loader and remain ordinary Kyma-callable functions at the language boundary.
