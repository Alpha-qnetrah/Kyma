#pragma once

#include <cstddef>
#include <string>

namespace kyna {

using SourceId = std::size_t;
inline constexpr SourceId UnknownSource = 0;

struct SourceSpan {
  SourceId source{UnknownSource};
  std::size_t startByte{0};
  std::size_t endByte{0};
  int line{1};
  int column{1};
  int endLine{1};
  int endColumn{1};

  constexpr SourceSpan() = default;
  constexpr SourceSpan(int startLine, int startColumn)
      : line(startLine), column(startColumn), endLine(startLine), endColumn(startColumn) {}
  constexpr SourceSpan(SourceId sourceId, std::size_t start, std::size_t end, int startLine,
                       int startColumn, int finishLine, int finishColumn)
      : source(sourceId), startByte(start), endByte(end), line(startLine), column(startColumn),
        endLine(finishLine), endColumn(finishColumn) {}

  [[nodiscard]] bool known() const { return source != UnknownSource || startByte != endByte; }
};

using SourceLocation = SourceSpan;

} // namespace kyna
