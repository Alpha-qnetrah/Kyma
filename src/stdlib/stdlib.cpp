#include "kyma/stdlib.hpp"
#include "kyma/behavior.hpp"
#include "kyma/interpreter.hpp"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <thread>
#if defined(__unix__) || defined(__APPLE__)
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace kyma {

namespace {
std::string httpGetText(const std::string &url) {
#if !defined(__unix__) && !defined(__APPLE__)
  throw KymaError({"httpGet is unavailable on this platform", {1, 1}, false});
#else
  if (url.rfind("http://", 0) != 0)
    throw KymaError(
        {"httpGet currently supports only http:// URLs (TLS is not included)", {1, 1}, false});
  auto rest = url.substr(7);
  auto slash = rest.find('/');
  auto hostPort = rest.substr(0, slash);
  auto path = slash == std::string::npos ? "/" : rest.substr(slash);
  std::string host = hostPort;
  std::string port = "80";
  auto colon = hostPort.rfind(':');
  if (colon != std::string::npos) {
    host = hostPort.substr(0, colon);
    port = hostPort.substr(colon + 1);
  }
  addrinfo hints{};
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  addrinfo *result = nullptr;
  if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0)
    throw KymaError({"could not resolve host '" + host + "'", {1, 1}, false});
  int fd = -1;
  for (auto *a = result; a; a = a->ai_next) {
    fd = socket(a->ai_family, a->ai_socktype, a->ai_protocol);
    if (fd >= 0 && connect(fd, a->ai_addr, a->ai_addrlen) == 0)
      break;
    if (fd >= 0) {
      close(fd);
      fd = -1;
    }
  }
  freeaddrinfo(result);
  if (fd < 0)
    throw KymaError({"could not connect to '" + host + "'", {1, 1}, false});
  std::string request =
      "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
  if (send(fd, request.data(), request.size(), 0) < 0) {
    close(fd);
    throw KymaError({"failed to send HTTP request", {1, 1}, false});
  }
  std::string response;
  char buffer[4096];
  for (;;) {
    auto n = recv(fd, buffer, sizeof(buffer), 0);
    if (n <= 0)
      break;
    response.append(buffer, static_cast<size_t>(n));
  }
  close(fd);
  auto split = response.find("\r\n\r\n");
  return split == std::string::npos ? response : response.substr(split + 4);
#endif
}
} // namespace

void installStandardLibrary(Interpreter &interpreter) {
  auto global = interpreter.globals();

  auto print = std::make_shared<Function>();
  print->native = true;
  print->nativeCall = [](const std::vector<Value> &a) {
    for (size_t i = 0; i < a.size(); ++i) {
      if (i)
        std::cout << ' ';
      std::cout << a[i].display();
    }
    std::cout << '\n';
    return Value();
  };
  global->define("print", Value(print), false);
  global->define("log", Value(print), false);
  auto console = interpreter.heap().allocate();
  console->fields["log"] = Value(print);
  auto consoleValue = Value(console);
  global->define("console", std::move(consoleValue), false);
  auto colorLog = std::make_shared<Function>();
  colorLog->native = true;
  colorLog->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 2 || !std::holds_alternative<std::string>(a[0].data) ||
        !std::holds_alternative<std::string>(a[1].data))
      throw KymaError({"logColor expects a color and message", {1, 1}, false});
    static const std::map<std::string, std::string> colors = {
        {"black", "30"},   {"red", "31"},  {"green", "32"}, {"yellow", "33"}, {"blue", "34"},
        {"magenta", "35"}, {"cyan", "36"}, {"white", "37"}, {"reset", "0"}};
    auto found = colors.find(std::get<std::string>(a[0].data));
    if (found == colors.end())
      throw KymaError({"unknown log color", {1, 1}, false});
    std::cout << "\033[" << found->second << "m" << std::get<std::string>(a[1].data) << "\033[0m\n";
    return Value();
  };
  global->define("logColor", Value(colorLog), false);
  auto raise = std::make_shared<Function>();
  raise->native = true;
  raise->nativeCall = [](const std::vector<Value> &a) -> Value {
    if (a.size() != 1)
      throw KymaError({"error expects one message", {1, 1}, false});
    throw KymaError({a[0].display(), {1, 1}, false});
  };
  global->define("error", Value(raise), false);
  auto type = std::make_shared<Function>();
  type->native = true;
  type->nativeCall = [](const std::vector<Value> &a) {
    return a.empty() ? Value(std::string("void")) : Value(a[0].typeName());
  };
  global->define("typeOf", Value(type), false);
  auto collect = std::make_shared<Function>();
  collect->native = true;
  collect->nativeCall = [&interpreter, global](const std::vector<Value> &) {
    interpreter.heap().collect({global.get(), interpreter.currentEnvironment().get()});
    return Value();
  };
  global->define("collectGarbage", Value(collect), false);
  auto stats = std::make_shared<Function>();
  stats->native = true;
  stats->nativeCall = [&interpreter, global](const std::vector<Value> &) {
    return Value(std::string("heap: ") + std::to_string(interpreter.heap().live()) + " live, " +
                 std::to_string(interpreter.heap().collections()) + " collections");
  };
  global->define("gcStats", Value(stats), false);
  auto length = std::make_shared<Function>();
  length->native = true;
  length->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 1)
      throw KymaError({"len expects one argument", {1, 1}, false});
    if (auto s = std::get_if<std::string>(&a[0].data))
      return Value(static_cast<int64_t>(s->size()));
    if (auto x = std::get_if<ArrayPtr>(&a[0].data))
      return Value(static_cast<int64_t>((*x)->elements.size()));
    if (auto o = std::get_if<ObjectPtr>(&a[0].data))
      return Value(static_cast<int64_t>((*o)->fields.size()));
    throw KymaError({"len requires a string, array, or object", {1, 1}, false});
  };
  global->define("len", Value(length), false);
  auto push = std::make_shared<Function>();
  push->native = true;
  push->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 2 || !std::holds_alternative<ArrayPtr>(a[0].data))
      throw KymaError({"push expects an array and a value", {1, 1}, false});
    std::get<ArrayPtr>(a[0].data)->elements.push_back(a[1]);
    return Value();
  };
  global->define("push", Value(push), false);
  auto pop = std::make_shared<Function>();
  pop->native = true;
  pop->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<ArrayPtr>(a[0].data))
      throw KymaError({"pop expects an array", {1, 1}, false});
    auto array = std::get<ArrayPtr>(a[0].data);
    if (array->elements.empty())
      return Value();
    auto v = array->elements.back();
    array->elements.pop_back();
    return v;
  };
  global->define("pop", Value(pop), false);
  auto keys = std::make_shared<Function>();
  keys->native = true;
  keys->nativeCall = [&interpreter, global](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<ObjectPtr>(a[0].data))
      throw KymaError({"keys expects an object", {1, 1}, false});
    auto array = interpreter.heap().allocateArray();
    for (const auto &[name, value] : std::get<ObjectPtr>(a[0].data)->fields)
      array->elements.emplace_back(name);
    return Value(array);
  };
  global->define("keys", Value(keys), false);
  auto read = std::make_shared<Function>();
  read->native = true;
  read->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<std::string>(a[0].data))
      throw KymaError({"readFile expects a path", {1, 1}, false});
    std::ifstream file(std::get<std::string>(a[0].data));
    if (!file)
      throw KymaError({"could not read file", {1, 1}, false});
    std::stringstream text;
    text << file.rdbuf();
    return Value(text.str());
  };
  global->define("readFile", Value(read), false);
  auto write = std::make_shared<Function>();
  write->native = true;
  write->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 2 || !std::holds_alternative<std::string>(a[0].data) ||
        !std::holds_alternative<std::string>(a[1].data))
      throw KymaError({"writeFile expects path and string content", {1, 1}, false});
    std::ofstream file(std::get<std::string>(a[0].data));
    if (!file)
      throw KymaError({"could not write file", {1, 1}, false});
    file << std::get<std::string>(a[1].data);
    return Value();
  };
  global->define("writeFile", Value(write), false);
  auto run = std::make_shared<Function>();
  run->native = true;
  run->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<std::string>(a[0].data))
      throw KymaError({"processRun expects a shell command string", {1, 1}, false});
    return Value(static_cast<int64_t>(std::system(std::get<std::string>(a[0].data).c_str())));
  };
  global->define("processRun", Value(run), false);
  global->define("build", Value(run), false);
  auto environment = std::make_shared<Function>();
  environment->native = true;
  environment->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<std::string>(a[0].data))
      throw KymaError({"processEnv expects a variable name", {1, 1}, false});
    auto *value = std::getenv(std::get<std::string>(a[0].data).c_str());
    return value ? Value(std::string(value)) : Value();
  };
  global->define("processEnv", Value(environment), false);
  auto sleep = std::make_shared<Function>();
  sleep->native = true;
  sleep->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<int64_t>(a[0].data))
      throw KymaError({"sleep expects milliseconds", {1, 1}, false});
    std::this_thread::sleep_for(std::chrono::milliseconds(std::get<int64_t>(a[0].data)));
    return Value();
  };
  global->define("sleep", Value(sleep), false);
  global->define("wait", Value(sleep), false);
  auto get = std::make_shared<Function>();
  get->native = true;
  get->nativeCall = [](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<std::string>(a[0].data))
      throw KymaError({"httpGet expects a URL string", {1, 1}, false});
    return Value(httpGetText(std::get<std::string>(a[0].data)));
  };
  global->define("httpGet", Value(get), false);
  global->define("fetch", Value(get), false);
}
} // namespace kyma
