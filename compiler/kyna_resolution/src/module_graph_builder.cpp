#include "kyna/modules/module_loader.hpp"

// Graph construction is localized in filesystem_module_loader.cpp. Keeping resolution and graph
// mutation together prevents callers from learning import-cache and traversal invariants.
