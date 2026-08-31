#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace kyna {

using DatabaseScalar =
    std::variant<std::nullptr_t, bool, std::int64_t, double, std::string>;
using DatabaseRow = std::map<std::string, DatabaseScalar>;

struct DatabaseRequest {
  std::string connectionString;
  std::string statement;
  std::vector<DatabaseScalar> parameters;
};

struct DatabaseResult {
  std::vector<DatabaseRow> rows;
  std::uint64_t affectedRows{0};
  std::string command;
};

enum class DatabaseFailurePhase { Configuration, Connect, Prepare, Execute, Decode };

struct DatabaseFailure {
  DatabaseFailurePhase phase{DatabaseFailurePhase::Execute};
  std::string nativeCode;
  std::string message;
  bool retryable{false};
};

[[nodiscard]] const char *databaseFailurePhaseName(DatabaseFailurePhase phase);

class DatabasePort {
public:
  virtual ~DatabasePort() = default;
  virtual std::optional<DatabaseResult> execute(const DatabaseRequest &request,
                                                DatabaseFailure &failure) = 0;
};

[[nodiscard]] std::shared_ptr<DatabasePort> productionDatabasePort();

} // namespace kyna
