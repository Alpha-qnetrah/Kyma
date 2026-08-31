#include "kyna/stdlib/database_library.hpp"

#include "kyna/execution/database_port.hpp"
#include "kyna/execution/tree_walk_engine.hpp"

namespace kyna {
namespace {

KynaError databaseError(const DatabaseFailure &failure) {
  Diagnostic diagnostic{"database " + std::string(databaseFailurePhaseName(failure.phase)) +
                            " error: " + failure.message,
                        {}, false, "KDB2001"};
  diagnostic.category = "database";
  diagnostic.causes.push_back({"postgresql", failure.nativeCode, failure.message});
  diagnostic.notes.push_back(failure.retryable
                                 ? "the database adapter classified this failure as retryable"
                                 : "the database adapter classified this failure as non-retryable");
  diagnostic.help =
      "verify the connection settings and SQL; use parameter placeholders such as $1 instead of concatenating values";
  return KynaError(diagnostic);
}

DatabaseScalar databaseScalar(const Value &value) {
  return std::visit(
      [](const auto &stored) -> DatabaseScalar {
        using T = std::decay_t<decltype(stored)>;
        if constexpr (std::is_same_v<T, std::nullptr_t> || std::is_same_v<T, bool> ||
                      std::is_same_v<T, std::int64_t> || std::is_same_v<T, double> ||
                      std::is_same_v<T, std::string>)
          return stored;
        else if constexpr (std::is_same_v<T, char>)
          return std::string(1, stored);
        else
          throw KynaError({"database parameters must be null, bool, number, string, or char",
                           {}, false, "KDB1002"});
      },
      value.data);
}

Value runtimeScalar(const DatabaseScalar &value) {
  return std::visit([](const auto &stored) { return Value(stored); }, value);
}

} // namespace

void installDatabaseLibrary(Interpreter &interpreter) {
  const auto capabilities = interpreter.runtimeCapabilities();
  auto databaseQuery = std::make_shared<Function>();
  databaseQuery->native = true;
  databaseQuery->nativeCall = [&interpreter, capabilities](const std::vector<Value> &arguments) {
    if (arguments.size() < 2 || arguments.size() > 3 ||
        !std::holds_alternative<std::string>(arguments[0].data) ||
        !std::holds_alternative<std::string>(arguments[1].data) ||
        (arguments.size() == 3 && !std::holds_alternative<ArrayPtr>(arguments[2].data)))
      throw KynaError(
          {"db.query expects a connection string, SQL string, and optional parameter array",
           {}, false, "KDB1001"});
    if (!capabilities.database)
      throw KynaError({"database capability is not available in this runtime", {}, false,
                       "KDB1000"});

    DatabaseRequest request;
    request.connectionString = std::get<std::string>(arguments[0].data);
    request.statement = std::get<std::string>(arguments[1].data);
    if (arguments.size() == 3)
      for (const auto &parameter : std::get<ArrayPtr>(arguments[2].data)->elements)
        request.parameters.push_back(databaseScalar(parameter));

    DatabaseFailure failure;
    const auto databaseResult = capabilities.database->execute(request, failure);
    if (!databaseResult)
      throw databaseError(failure);

    auto rows = interpreter.heap().allocateArray();
    for (const auto &databaseRow : databaseResult->rows) {
      auto row = interpreter.heap().allocate();
      for (const auto &[column, value] : databaseRow)
        row->fields.insert_or_assign(column, runtimeScalar(value));
      rows->elements.emplace_back(row);
    }
    auto result = interpreter.heap().allocate();
    result->fields["rows"] = Value(rows);
    result->fields["affectedRows"] =
        Value(static_cast<std::int64_t>(databaseResult->affectedRows));
    result->fields["command"] = Value(databaseResult->command);
    return Value(result);
  };

  auto database = interpreter.heap().allocate();
  database->fields["query"] = Value(databaseQuery);
  database->fields["execute"] = Value(databaseQuery);
  interpreter.globals()->define("db", Value(database), false);
}

} // namespace kyna
