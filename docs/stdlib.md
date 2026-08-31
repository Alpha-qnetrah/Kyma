# Standard library

The v0.3 runtime exposes native functions through the global environment rather than special-casing them in the parser:

- `print(value, ...)`, `log(value, ...)`, `console.log(value, ...)`, `logColor(color, message)`, and `typeOf(value)`
- `len`, `push`, `pop`, and `keys` for arrays, strings, and closed objects
- `readFile(path)`, `writeFile(path, content)`, `readJsonFile(path)`, and `writeJsonFile(path, value)`
- `createDirectory(path)`, `fileExists(path)`, `removePath(path)`, and `listDirectory(path)`
- `fs.read`, `fs.write`, `fs.readJson`, `fs.writeJson`, `fs.createDirectory`, `fs.exists`, `fs.remove`, and `fs.list`
- `processRun(command)`, `build(command)`, `processEnv(name)`, `sleep(milliseconds)`, and `wait(milliseconds)`
- `httpGet(url)` for a raw response body and `fetch(url)` for a response object with `ok`, `status`, `url`, `text()`, and `json()`
- `jsonParse`, `jsonStringify`, `process.json`, `process.stringify`, `process.run`, and `process.env`
- `filter(array, predicate)`, `bubbleSort(array, comparator?)`, `sort`, and `call(function, args?)`
- `createApiStore(records)` for an in-memory CRUD store with `list`, `get`, `create`, `update`, and `remove`
- `error(message)` for language errors caught by `try`/`catch`
- `collectGarbage()` and `gcStats()` for heap diagnostics

Filesystem, process, networking, and sleeping operations call injected `RuntimeCapabilities`; deterministic embedders can replace every host adapter. The production adapter handles plain HTTP directly and delegates HTTPS plus POST/PUT/PATCH/DELETE requests to the system `curl` executable. Curl requests use HTTP/1.1, a Kyna user agent, a 10-second connect timeout, a 30-second overall timeout, and two retries for transient transport failures. Standard `HTTPS_PROXY`, `HTTP_PROXY`, and `ALL_PROXY` environment variables are honored by curl when a network or VPN requires a proxy. Streaming, async I/O, and cancellation remain future work. Process execution uses the host shell and should be treated as an explicitly trusted capability.

`examples/fake_api_store.kyna` uses the real `https://fakestoreapi.com/products` endpoints and writes the retrieved collection to `fake-store-output/products.json`. Fake Store API mutations are intentionally simulated by that service: responses demonstrate CRUD request/JSON behavior but do not permanently modify its database.

`examples/weather_api.kyna` uses the keyless Open-Meteo forecast endpoint to smoke-test HTTPS and JSON parsing, then writes the response to `weather-output/current.json`.
