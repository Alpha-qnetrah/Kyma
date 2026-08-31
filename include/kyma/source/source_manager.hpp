#pragma once

#include "kyma/source/source_file.hpp"
#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

namespace kyma {

class SourceManager {
public:
  SourceId add(std::filesystem::path path, std::string text);
  std::optional<SourceId> load(const std::filesystem::path &path, std::string &error);
  [[nodiscard]] const SourceFile *find(SourceId id) const;
  [[nodiscard]] std::string_view line(SourceId id, int oneBasedLine) const;
  [[nodiscard]] SourceSpan span(SourceId id, std::size_t start, std::size_t end) const;

private:
  struct ManagedSource {
    SourceFile file;
    std::vector<std::size_t> lineStarts;
  };
  std::vector<ManagedSource> sources;
};

} // namespace kyma
