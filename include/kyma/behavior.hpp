#pragma once
#include <string>
#include <vector>

namespace kyma {
// Shared modifier policy used by class validation and runtime dispatch.
bool hasModifier(const std::vector<std::string> &modifiers, const std::string &modifier);
} // namespace kyma
