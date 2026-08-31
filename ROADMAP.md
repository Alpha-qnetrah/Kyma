# Kyna roadmap

## Mandatory 1.0 gates

- Replace recursive syntax ownership with an arena and stable typed IDs.
- Add resolved HIR, typed HIR, control-flow analysis, explicit MIR, and complete syntax-to-bytecode lowering.
- Move all execution to the bytecode VM and remove the temporary tree-walk compatibility engine.
- Implement exceptions, async functions, futures, cancellation, and the single-threaded event loop in bytecode.
- Complete generational GC, write barriers, heap path/dominator inspection, and async root tracing.
- Complete Unicode text, typed JSON decoding, collection iterators, process controls, HTTP server, DNS/IP, and TCP/UDP APIs.
- Deliver LSP, DAP, formatter, linter command, test runner, documentation tool, and path/Git package manager.
- Pass clean-machine Windows, Linux, and macOS installation tests with signed release artifacts.

## Deferred beyond 1.0

- Native-code compilation and JIT execution.
- Shared-memory multithreading.
- A central package registry.
- A first-party GUI framework.
