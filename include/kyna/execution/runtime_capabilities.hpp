#pragma once

#include <chrono>
#include <filesystem>
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

class NetworkPort {
public:
  virtual ~NetworkPort() = default;
  virtual std::optional<std::string> get(const std::string &url, std::string &error) = 0;
  virtual std::optional<std::string> request(const std::string &method, const std::string &url,
                                             const std::optional<std::string> &body,
                                             std::string &error) {
    if (method == "GET" && !body)
      return get(url, error);
    error = "network adapter does not support " + method + " requests";
    return std::nullopt;
  }
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
};

RuntimeCapabilities productionRuntimeCapabilities();

} // namespace kyna
