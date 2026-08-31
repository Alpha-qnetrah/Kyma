# Modules

Kyma v0.2 uses immutable namespace imports:

```kyma
import "./math.ky" as math;
export func add(a: int, b: int): int { return a + b; }
```

- one source file defines a module identity;
- `export` adds a declaration to the module's public table;
- `import "path" as name` loads and caches a module, analyzes it once, then exposes only exports;
- resolution uses the importing file's directory first, followed by configured library roots;
- cycles are diagnosed with an import stack;
- module initialization runs once and in dependency order.

Imports must precede other top-level declarations. Only named top-level declarations can be exported. Namespace reads are live, namespace writes are forbidden, and private declarations are not visible. Resolution canonicalizes the importer-relative candidate before checking repeated `--module-path` roots. Cycles report the full filename chain. Dependencies initialize once in postorder.
