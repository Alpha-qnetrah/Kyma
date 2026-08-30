#include "kyma/types.hpp"

namespace kyma {
std::string TypeRef::str() const {
  std::string result = name;
  if (nullable)
    result += "?";
  for (const auto &type : unionTypes)
    result += " | " + type.str();
  return result;
}
} // namespace kyma
