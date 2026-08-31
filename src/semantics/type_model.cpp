#include "kyna/types.hpp"

namespace kyna {
std::string TypeRef::str() const {
  std::string result = name;
  if (nullable)
    result += "?";
  for (const auto &type : unionTypes)
    result += " | " + type.str();
  return result;
}
} // namespace kyna
