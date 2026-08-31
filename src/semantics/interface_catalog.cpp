#include "kyma/interfaces.hpp"

namespace kyma {
bool InterfaceCatalog::declareInterface(const InterfaceDecl &declaration) {
  return declarations.emplace(declaration.name, declaration).second;
}
const InterfaceDecl *InterfaceCatalog::find(const std::string &name) const {
  auto found = declarations.find(name);
  return found == declarations.end() ? nullptr : &found->second;
}
void InterfaceCatalog::clear() { declarations.clear(); }
} // namespace kyma
