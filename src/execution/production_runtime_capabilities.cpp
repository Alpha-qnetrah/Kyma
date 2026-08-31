#include "kyma/execution/runtime_capabilities.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#if defined(__unix__) || defined(__APPLE__)
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace kyma {
namespace {

class LocalFileSystem final : public FileSystemPort {
public:
  std::optional<std::string> read(const std::filesystem::path &path, std::string &error) override {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
      error = "could not read file '" + path.string() + "'";
      return std::nullopt;
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
  }
  bool write(const std::filesystem::path &path, const std::string &contents,
             std::string &error) override {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
      error = "could not write file '" + path.string() + "'";
      return false;
    }
    file << contents;
    return static_cast<bool>(file);
  }
  bool createDirectories(const std::filesystem::path &path, std::string &error) override {
    std::error_code failure;
    std::filesystem::create_directories(path, failure);
    if (failure) {
      error = "could not create directory '" + path.string() + "': " + failure.message();
      return false;
    }
    return std::filesystem::is_directory(path, failure) && !failure;
  }
  bool exists(const std::filesystem::path &path, std::string &error) override {
    std::error_code failure;
    const bool found = std::filesystem::exists(path, failure);
    if (failure)
      error = "could not inspect path '" + path.string() + "': " + failure.message();
    return found;
  }
  bool remove(const std::filesystem::path &path, std::string &error) override {
    std::error_code failure;
    const bool removed = std::filesystem::remove(path, failure);
    if (failure)
      error = "could not remove path '" + path.string() + "': " + failure.message();
    return removed;
  }
  std::optional<std::vector<std::string>> list(const std::filesystem::path &path,
                                               std::string &error) override {
    std::error_code failure;
    std::filesystem::directory_iterator entries(path, failure);
    if (failure) {
      error = "could not list directory '" + path.string() + "': " + failure.message();
      return std::nullopt;
    }
    std::vector<std::string> names;
    for (const auto &entry : entries)
      names.push_back(entry.path().filename().string());
    std::sort(names.begin(), names.end());
    return names;
  }
};

class LocalProcess final : public ProcessPort {
public:
  int run(const std::string &command) override { return std::system(command.c_str()); }
  std::optional<std::string> environment(const std::string &name) override {
    const auto *value = std::getenv(name.c_str());
    return value ? std::optional<std::string>(value) : std::nullopt;
  }
};

class SystemClock final : public ClockPort {
public:
  void sleep(std::chrono::milliseconds duration) override { std::this_thread::sleep_for(duration); }
};

class PlainHttpNetwork final : public NetworkPort {
public:
  std::optional<std::string> get(const std::string &url, std::string &error) override {
    return request("GET", url, std::nullopt, error);
  }

  std::optional<std::string> request(const std::string &method, const std::string &url,
                                     const std::optional<std::string> &body,
                                     std::string &error) override {
    if (url == "mock://kyma/users")
      return std::string(
          R"([{"id":1,"name":"Ada","active":true},{"id":2,"name":"Linus","active":false},{"id":3,"name":"Grace","active":true}])");
#if defined(_WIN32)
    if (!url.starts_with("http://") && !url.starts_with("https://")) {
      error = "unsupported URL scheme";
      return std::nullopt;
    }
    return curlRequest(method, url, body, error);
#elif !defined(__unix__) && !defined(__APPLE__)
    error = "network requests are unavailable on this platform";
    return std::nullopt;
#else
    if (url.starts_with("https://") || method != "GET" || body)
      return curlRequest(method, url, body, error);
    if (!url.starts_with("http://")) {
      error = "unsupported URL scheme";
      return std::nullopt;
    }
    auto rest = url.substr(7);
    auto slash = rest.find('/');
    auto hostPort = rest.substr(0, slash);
    auto requestPath = slash == std::string::npos ? "/" : rest.substr(slash);
    std::string host = hostPort;
    std::string port = "80";
    if (const auto colon = hostPort.rfind(':'); colon != std::string::npos) {
      host = hostPort.substr(0, colon);
      port = hostPort.substr(colon + 1);
    }
    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    addrinfo *addresses = nullptr;
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &addresses) != 0) {
      error = "could not resolve host '" + host + "'";
      return std::nullopt;
    }
    int socketDescriptor = -1;
    for (auto *address = addresses; address; address = address->ai_next) {
      socketDescriptor = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
      if (socketDescriptor >= 0 &&
          connect(socketDescriptor, address->ai_addr, address->ai_addrlen) == 0)
        break;
      if (socketDescriptor >= 0)
        close(socketDescriptor);
      socketDescriptor = -1;
    }
    freeaddrinfo(addresses);
    if (socketDescriptor < 0) {
      error = "could not connect to '" + host + "'";
      return std::nullopt;
    }
    const std::string request =
        "GET " + requestPath + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    if (send(socketDescriptor, request.data(), request.size(), 0) < 0) {
      close(socketDescriptor);
      error = "failed to send HTTP request";
      return std::nullopt;
    }
    std::string response;
    char buffer[4096];
    for (;;) {
      const auto count = recv(socketDescriptor, buffer, sizeof(buffer), 0);
      if (count <= 0)
        break;
      response.append(buffer, static_cast<std::size_t>(count));
    }
    close(socketDescriptor);
    const auto split = response.find("\r\n\r\n");
    return split == std::string::npos ? response : response.substr(split + 4);
#endif
  }

private:
#if defined(_WIN32)
  static std::string windowsShellQuote(const std::string &value) {
    // CommandLineToArgvW-compatible quoting for the arguments passed to the
    // Windows curl executable. This keeps URLs and JSON request bodies intact.
    std::string quoted{"\""};
    std::size_t backslashes = 0;
    for (const char character : value) {
      if (character == '\\') {
        ++backslashes;
      } else if (character == '"') {
        quoted.append(backslashes * 2 + 1, '\\');
        quoted.push_back('"');
        backslashes = 0;
      } else {
        quoted.append(backslashes, '\\');
        quoted.push_back(character);
        backslashes = 0;
      }
    }
    quoted.append(backslashes * 2, '\\');
    quoted.push_back('"');
    return quoted;
  }

  static std::optional<std::string> curlRequest(const std::string &method, const std::string &url,
                                                const std::optional<std::string> &body,
                                                std::string &error) {
    std::string command =
        "curl --fail-with-body --silent --show-error --location --retry 2 "
        "--retry-all-errors --retry-delay 1 --connect-timeout 10 --max-time 30 "
        "--user-agent Kyma/0.2.2 --http1.1 --request " + windowsShellQuote(method);
    if (body)
      command += " --header " + windowsShellQuote("Content-Type: application/json") +
                 " --data " + windowsShellQuote(*body);
    command += " " + windowsShellQuote(url) + " 2>&1";

    FILE *pipe = _popen(command.c_str(), "r");
    if (!pipe) {
      error = "could not start curl; install curl and ensure it is on PATH";
      return std::nullopt;
    }
    std::string response;
    char buffer[4096];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr)
      response.append(buffer);
    const int status = _pclose(pipe);
    if (status != 0) {
      error = response.empty() ? "HTTP request failed" : response;
      return std::nullopt;
    }
    return response;
  }
#endif
#if defined(__unix__) || defined(__APPLE__)
  static std::optional<std::string> curlRequest(const std::string &method, const std::string &url,
                                                const std::optional<std::string> &body,
                                                std::string &error) {
    int outputPipe[2];
    if (pipe(outputPipe) != 0) {
      error = "could not create HTTPS response pipe";
      return std::nullopt;
    }
    const auto child = fork();
    if (child < 0) {
      close(outputPipe[0]);
      close(outputPipe[1]);
      error = "could not start HTTPS client";
      return std::nullopt;
    }
    if (child == 0) {
      close(outputPipe[0]);
      dup2(outputPipe[1], STDOUT_FILENO);
      dup2(outputPipe[1], STDERR_FILENO);
      close(outputPipe[1]);
      std::vector<std::string> arguments{"curl",
                                         "--fail-with-body",
                                         "--silent",
                                         "--show-error",
                                         "--location",
                                         "--retry",
                                         "2",
                                         "--retry-all-errors",
                                         "--retry-delay",
                                         "1",
                                         "--connect-timeout",
                                         "10",
                                         "--max-time",
                                         "30",
                                         "--user-agent",
                                         "Kyma/0.2.2",
                                         "--http1.1",
                                         "--request",
                                         method};
      if (body) {
        arguments.emplace_back("--header");
        arguments.emplace_back("Content-Type: application/json");
        arguments.emplace_back("--data");
        arguments.push_back(*body);
      }
      arguments.push_back(url);
      std::vector<char *> rawArguments;
      rawArguments.reserve(arguments.size() + 1);
      for (auto &argument : arguments)
        rawArguments.push_back(argument.data());
      rawArguments.push_back(nullptr);
      execvp("curl", rawArguments.data());
      _exit(127);
    }
    close(outputPipe[1]);
    std::string response;
    char buffer[4096];
    for (;;) {
      const auto count = read(outputPipe[0], buffer, sizeof(buffer));
      if (count <= 0)
        break;
      response.append(buffer, static_cast<std::size_t>(count));
    }
    close(outputPipe[0]);
    int status = 0;
    waitpid(child, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      error = response.empty() ? "HTTPS request failed" : response;
      return std::nullopt;
    }
    return response;
  }
#endif
};

} // namespace

RuntimeCapabilities productionRuntimeCapabilities() {
  return {std::make_shared<LocalFileSystem>(), std::make_shared<LocalProcess>(),
          std::make_shared<PlainHttpNetwork>(), std::make_shared<SystemClock>()};
}

} // namespace kyma
