#pragma once

#include "kyna/hir/hir_program.hpp"
#include <string>

namespace kyna {

[[nodiscard]] std::string renderHir(const HirProgram &program);

} // namespace kyna
