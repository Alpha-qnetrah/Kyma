#pragma once

namespace kyna {
class Interpreter;
// Installs the trusted host adapters exposed by the v0.1 standard library.
void installStandardLibrary(Interpreter &interpreter);
} // namespace kyna
