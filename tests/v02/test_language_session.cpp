#include "kyma/language/language_session.hpp"
#include "kyma/lexing/tokenizer.hpp"
#include "kyma/parsing/module_parser.hpp"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

class RecordingFiles final : public kyma::FileSystemPort {
public:
  std::string contents;
  int reads{0};
  int writes{0};
  int directories{0};
  int removals{0};
  std::optional<std::string> read(const std::filesystem::path &, std::string &) override {
    ++reads;
    return contents;
  }
  bool write(const std::filesystem::path &, const std::string &value, std::string &) override {
    ++writes;
    contents = value;
    return true;
  }
  bool createDirectories(const std::filesystem::path &, std::string &) override {
    ++directories;
    return true;
  }
  bool exists(const std::filesystem::path &, std::string &) override { return true; }
  bool remove(const std::filesystem::path &, std::string &) override {
    ++removals;
    return true;
  }
  std::optional<std::vector<std::string>> list(const std::filesystem::path &,
                                               std::string &) override {
    return std::vector<std::string>{"products.json"};
  }
};

class RecordingProcesses final : public kyma::ProcessPort {
public:
  int runs{0};
  int run(const std::string &) override {
    ++runs;
    return 0;
  }
  std::optional<std::string> environment(const std::string &) override {
    return std::string("test");
  }
};

class RecordingNetwork final : public kyma::NetworkPort {
public:
  int requests{0};
  std::string lastMethod;
  std::optional<std::string> lastBody;
  std::optional<std::string> get(const std::string &, std::string &) override {
    ++requests;
    return std::string("[3,1,2]");
  }
  std::optional<std::string> request(const std::string &method, const std::string &,
                                     const std::optional<std::string> &body,
                                     std::string &) override {
    ++requests;
    lastMethod = method;
    lastBody = body;
    if (method == "POST")
      return std::string("{\"id\":31,\"title\":\"created\"}");
    return std::string("[3,1,2]");
  }
};

class RecordingClock final : public kyma::ClockPort {
public:
  int sleeps{0};
  void sleep(std::chrono::milliseconds) override { ++sleeps; }
};

void writeSource(const std::filesystem::path &path, const std::string &source) {
  std::ofstream file(path, std::ios::binary);
  assert(file);
  file << source;
  assert(file);
}

bool hasCode(const std::vector<kyma::Diagnostic> &diagnostics, const std::string &code) {
  for (const auto &diagnostic : diagnostics)
    if (diagnostic.code == code)
      return true;
  return false;
}

} // namespace

int main() {
  const kyma::SourceFile comments{7, "comments.ky", "# π\nlet value: int = 1;"};
  auto lexed = kyma::tokenize(comments);
  assert(lexed.ok());
  assert(lexed.tokens.front().kind == kyma::TokenKind::Let);
  assert(lexed.tokens.front().location.startByte == 5);
  assert(lexed.tokens.front().location.line == 2);
  assert(lexed.tokens.front().location.column == 1);

  const kyma::SourceFile malformed{8, "malformed.ky", "let first = ; let second = ;"};
  auto malformedTokens = kyma::tokenize(malformed);
  auto recovered = kyma::parseModule(malformed, std::move(malformedTokens.tokens));
  assert(!recovered.ok());
  assert(recovered.diagnostics.size() == 2);

  kyma::LanguageSession session;
  assert(session
             .checkSource("interface-ok.ky", "intf Named { name: str; } "
                                             "class User implements Named { public name: str; }")
             .ok());
  assert(!session
              .checkSource("interface-bad.ky",
                           "intf Named { name: str; } class User implements Named { }")
              .ok());

  auto unprotected = session.checkSource("unprotected.ky",
                                         "set response = fetch(\"http://example.test/products\");");
  assert(unprotected.ok());
  assert(hasCode(unprotected.diagnostics, "K2601"));
  assert(hasCode(unprotected.diagnostics, "K2603"));
  const auto protectedFetch = session.checkSource(
      "protected.ky", "try { set response = fetch(\"https://example.test/products\"); } "
                      "catch (message) { print(message); }");
  assert(protectedFetch.ok());
  assert(!hasCode(protectedFetch.diagnostics, "K2601"));
  const auto emptyCatch =
      session.checkSource("empty-catch.ky", "try { error(\"failure\"); } catch (message) { }");
  assert(emptyCatch.ok());
  assert(hasCode(emptyCatch.diagnostics, "K2602"));

  auto files = std::make_shared<RecordingFiles>();
  auto processes = std::make_shared<RecordingProcesses>();
  auto network = std::make_shared<RecordingNetwork>();
  auto clock = std::make_shared<RecordingClock>();
  kyma::LanguageSessionOptions deterministicOptions;
  deterministicOptions.capabilities = {files, processes, network, clock};
  kyma::LanguageSession deterministicSession(std::move(deterministicOptions));
  assert(deterministicSession
             .runSource("capabilities.ky",
                        "writeFile(\"memory\", \"value\"); readFile(\"memory\"); "
                        "processRun(\"command\"); processEnv(\"NAME\"); sleep(1); "
                        "httpGet(\"http://example.test\"); "
                        "set response = fetch(\"http://example.test\"); "
                        "set values = response.json(); set ordered = sort(values); "
                        "if (ordered[0] != 1) { error(\"sort failed\"); } "
                        "set metadata = process.json(\"{\\\"ready\\\":true}\"); "
                        "if (metadata.ready != true) { error(\"json failed\"); } "
                        "set createdResponse = fetch(\"https://example.test/products\", "
                        "{ method: \"POST\", body: \"{\\\"title\\\":\\\"created\\\"}\" }); "
                        "set created = createdResponse.json(); "
                        "if (created.id != 31) { error(\"POST failed\"); } "
                        "try { createDirectory(\"cache\"); "
                        "writeJsonFile(\"cache/products.json\", created); "
                        "set saved = readJsonFile(\"cache/products.json\"); "
                        "if (saved.id != 31 || !fileExists(\"cache/products.json\")) { "
                        "error(\"JSON file failed\"); } "
                        "set entries = listDirectory(\"cache\"); "
                        "if (entries[0] != \"products.json\") { error(\"list failed\"); } "
                        "removePath(\"cache/products.json\"); "
                        "} catch (message) { error(message); }")
             .ok());
  assert(files->writes == 2 && files->reads == 2);
  assert(files->contents == "{\"id\":31,\"title\":\"created\"}");
  assert(files->directories == 1 && files->removals == 1);
  assert(processes->runs == 1);
  assert(network->requests == 3);
  assert(network->lastMethod == "POST");
  assert(network->lastBody == "{\"title\":\"created\"}");
  assert(clock->sleeps == 1);
  assert(!session
              .checkSource("private.ky", "class Secret { value: int; } let secret = new Secret(); "
                                         "let leaked = secret.value;")
              .ok());
  assert(
      !session.checkSource("final.ky", "final class Base { } class Derived extends Base { }").ok());
  assert(!session
              .checkSource("override.ky",
                           "class Base { public func value(): int { return 1; } } "
                           "class Derived extends Base { public func value(): int { return 2; } }")
              .ok());
  assert(!session
              .checkSource("abstract.ky",
                           "abstract class Base { public abstract func value(): int; } "
                           "class Derived extends Base { }")
              .ok());

  const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
  const auto directory =
      std::filesystem::temp_directory_path() / ("kyma-v02-language-test-" + std::to_string(nonce));
  std::filesystem::create_directories(directory);
  writeSource(directory / "math.ky", "export func add(a: int, b: int): int { return a + b; } "
                                     "func hidden(): int { return 9; }");
  writeSource(directory / "main.ky",
              "import \"./math.ky\" as math; let answer: int = math.add(20, 22);");
  kyma::LanguageSession moduleSession;
  assert(moduleSession.check(directory / "main.ky").ok());
  assert(moduleSession.run(directory / "main.ky").ok());

  auto moduleFiles = std::make_shared<RecordingFiles>();
  kyma::LanguageSessionOptions cachedModuleOptions;
  cachedModuleOptions.capabilities = {moduleFiles, processes, network, clock};
  kyma::LanguageSession cachedModuleSession(std::move(cachedModuleOptions));
  writeSource(directory / "side-effect.ky",
              "export set value = 1; writeFile(\"initialization\", \"once\");");
  writeSource(directory / "cached-main.ky", "import \"./side-effect.ky\" as side;");
  assert(cachedModuleSession.run(directory / "cached-main.ky").ok());
  assert(cachedModuleSession.run(directory / "cached-main.ky").ok());
  assert(moduleFiles->writes == 1);

  auto overlay = moduleSession.checkSourceAtPath(
      directory / "main.ky", "import \"./math.ky\" as math; let answer: int = math.add(1, 2);");
  assert(overlay.ok());

  writeSource(directory / "private-import.ky",
              "import \"./math.ky\" as math; let answer = math.hidden();");
  auto privateImport = moduleSession.check(directory / "private-import.ky");
  assert(!privateImport.ok());
  assert(privateImport.diagnostics.front().message.find("no exported member") != std::string::npos);

  writeSource(directory / "cycle-a.ky", "import \"./cycle-b.ky\" as b;");
  writeSource(directory / "cycle-b.ky", "import \"./cycle-a.ky\" as a;");
  auto cycle = moduleSession.check(directory / "cycle-a.ky");
  assert(!cycle.ok());
  assert(hasCode(cycle.diagnostics, "K4002"));

  const auto persistedDirectory = directory / "persisted";
  const auto persistedFile = persistedDirectory / "products.json";
  const auto persistenceSource =
      "try { createDirectory(\"" + persistedDirectory.string() + "\"); writeJsonFile(\"" +
      persistedFile.string() + "\", { id: 1, title: \"saved\" }); set saved = readJsonFile(\"" +
      persistedFile.string() + "\"); if (saved.id != 1 || !fileExists(\"" + persistedFile.string() +
      "\")) { error(\"persistence failed\"); } set entries = listDirectory(\"" +
      persistedDirectory.string() +
      "\"); if (entries[0] != \"products.json\") { error(\"listing failed\"); } } "
      "catch (message) { error(message); }";
  kyma::LanguageSession productionFileSession;
  assert(productionFileSession.runSource("persistence.ky", persistenceSource).ok());
  assert(std::filesystem::is_regular_file(persistedFile));

  std::filesystem::remove_all(directory);
}
