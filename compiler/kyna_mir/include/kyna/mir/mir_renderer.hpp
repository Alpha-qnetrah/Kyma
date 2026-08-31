#pragma once

#include "kyna/mir/mir_program.hpp"
#include <string>

namespace kyna {

[[nodiscard]] std::string renderMir(const MirProgram &program);

} // namespace kyna
