#pragma once

#include "kyna/source/source_span.hpp"
#include <filesystem>
#include <string>

namespace kyna {

struct SourceFile {
  SourceId id{UnknownSource};
  std::filesystem::path path;
  std::string text;

  [[nodiscard]] std::string displayName() const {
    return path.empty() ? std::string("<source>") : path.string();
  }
};

} // namespace kyna
