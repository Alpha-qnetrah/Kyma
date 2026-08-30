#include "kyma/behavior.hpp"
#include <algorithm>

namespace kyma {
bool hasModifier(const std::vector<std::string> &modifiers, const std::string &modifier) {
  return std::find(modifiers.begin(), modifiers.end(), modifier) != modifiers.end();
}
} // namespace kyma
