#pragma once
#include "kyna/syntax/legacy_syntax_handles.hpp"
#include <map>
#include <string>

namespace kyna {
// Compile-time catalog for interface declarations. Runtime instances do not
// inherit interface state; conformance remains a validation concern.
class InterfaceCatalog {
public:
  bool declareInterface(const InterfaceDecl &declaration);
  const InterfaceDecl *find(const std::string &name) const;
  void clear();

private:
  std::map<std::string, InterfaceDecl> declarations;
};
} // namespace kyna
