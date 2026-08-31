#pragma once

#include "kyna/bytecode/bytecode_module.hpp"
#include "kyna/diagnostics/diagnostic.hpp"
#include "kyna/execution/runtime_value.hpp"
#include "kyna/memory/tracing_heap.hpp"
#include <utility>
#include <vector>

namespace kyna {

struct BytecodeExecutionResult {
  RuntimeValue value;
  std::vector<Diagnostic> diagnostics;
  HeapStats heapStats;
  BytecodeExecutionResult(RuntimeValue result = {},
                          std::vector<Diagnostic> failures = {},
                          HeapStats statistics = {})
      : value(std::move(result)), diagnostics(std::move(failures)), heapStats(statistics) {}
  [[nodiscard]] bool ok() const { return diagnostics.empty(); }
};

class BytecodeVirtualMachine {
public:
  [[nodiscard]] BytecodeExecutionResult execute(const BytecodeModule &module) const;
};

} // namespace kyna
