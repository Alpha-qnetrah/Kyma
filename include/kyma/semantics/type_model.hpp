#pragma once

#include <string>
#include <vector>

namespace kyma {

struct TypeRef {
  std::string name{"void"};
  bool nullable{false};
  std::vector<TypeRef> unionTypes;
  std::string str() const;
};

} // namespace kyma
