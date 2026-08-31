#pragma once

#include "kyna/execution/database_port.hpp"

#include <chrono>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace kyna {

class FileSystemPort {
public:
  virtual ~FileSystemPort() = default;
  virtual std::optional<std::string> read(const std::filesystem::path &, std::string &error) = 0;
  virtual bool write(const std::filesystem::path &, const std::string &, std::string &error) = 0;
  virtual bool createDirectories(const std::filesystem::path &, std::string &error) {
    error = "filesystem adapter does not support creating directories";
    return false;
  }
  virtual bool exists(const std::filesystem::path &, std::string &error) {
    error = "filesystem adapter does not support existence checks";
    return false;
  }
  virtual bool remove(const std::filesystem::path &, std::string &error) {
    error = "filesystem adapter does not support removing paths";
    return false;
  }
  virtual std::optional<std::vector<std::string>> list(const std::filesystem::path &,
                                                       std::string &error) {
    error = "filesystem adapter does not support listing directories";
    return std::nullopt;
  }
};

class ProcessPort {
public:
  virtual ~ProcessPort() = default;
  virtual int run(const std::string &command) = 0;
  virtual std::optional<std::string> environment(const std::string &name) = 0;
};

enum class NetworkFailurePhase { Dns, Connect, Tls, Send, Receive, Timeout, Http, Transfer };

struct NetworkRequest {
  std::string method{"GET"};
  std::string url;
  std::optional<std::string> body;
  std::map<std::string, std::string> headers;
  std::chrono::milliseconds timeout{30000};
};

struct NetworkResponse {
  long status{0};
  std::string body;
  std::string effectiveUrl;
  std::map<std::string, std::string> headers;
  [[nodiscard]] bool ok() const { return status >= 200 && status < 300; }
};

struct NetworkFailure {
  NetworkFailurePhase phase{NetworkFailurePhase::Transfer};
  int nativeCode{0};
  std::string message;
  bool retryable{false};
};

[[nodiscard]] const char *networkFailurePhaseName(NetworkFailurePhase phase);

class NetworkPort {
public:
  virtual ~NetworkPort() = default;
  virtual std::optional<NetworkResponse> send(const NetworkRequest &request,
                                              NetworkFailure &failure) = 0;
};

class ClockPort {
public:
  virtual ~ClockPort() = default;
  virtual void sleep(std::chrono::milliseconds duration) = 0;
};

struct RuntimeCapabilities {
  std::shared_ptr<FileSystemPort> files;
  std::shared_ptr<ProcessPort> processes;
  std::shared_ptr<NetworkPort> network;
  std::shared_ptr<ClockPort> clock;
  std::shared_ptr<DatabasePort> database;
};

RuntimeCapabilities productionRuntimeCapabilities();

} // namespace kyna
