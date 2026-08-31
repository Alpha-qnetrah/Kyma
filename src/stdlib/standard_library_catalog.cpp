#include "json_value_codec.hpp"
#include "kyma/behavior.hpp"
#include "kyma/execution/runtime_capabilities.hpp"
#include "kyma/interpreter.hpp"
#include "kyma/stdlib.hpp"
#include <algorithm>
#include <cctype>
#include <map>

namespace kyma {

void installStandardLibrary(Interpreter &interpreter) {
  auto global = interpreter.globals();
  auto capabilities = interpreter.runtimeCapabilities();

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
  collect->nativeCall = [&interpreter](const std::vector<Value> &) {
    interpreter.heap().collect(interpreter.rootEnvironments());
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
  read->nativeCall = [capabilities](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<std::string>(a[0].data))
      throw KymaError({"readFile expects a path", {1, 1}, false});
    std::string error;
    auto contents = capabilities.files->read(std::get<std::string>(a[0].data), error);
    if (!contents)
      throw KymaError({std::move(error), {1, 1}, false});
    return Value(std::move(*contents));
  };
  global->define("readFile", Value(read), false);
  auto write = std::make_shared<Function>();
  write->native = true;
  write->nativeCall = [capabilities](const std::vector<Value> &a) {
    if (a.size() != 2 || !std::holds_alternative<std::string>(a[0].data) ||
        !std::holds_alternative<std::string>(a[1].data))
      throw KymaError({"writeFile expects path and string content", {1, 1}, false});
    std::string error;
    if (!capabilities.files->write(std::get<std::string>(a[0].data),
                                   std::get<std::string>(a[1].data), error))
      throw KymaError({std::move(error), {1, 1}, false});
    return Value();
  };
  global->define("writeFile", Value(write), false);

  auto createDirectory = std::make_shared<Function>();
  createDirectory->native = true;
  createDirectory->nativeCall = [capabilities](const std::vector<Value> &arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0].data))
      throw KymaError({"createDirectory expects one path", {1, 1}, false, "K5200"});
    std::string error;
    if (!capabilities.files->createDirectories(std::get<std::string>(arguments[0].data), error))
      throw KymaError({std::move(error), {1, 1}, false, "K5200"});
    return Value(true);
  };
  global->define("createDirectory", Value(createDirectory), false);

  auto fileExists = std::make_shared<Function>();
  fileExists->native = true;
  fileExists->nativeCall = [capabilities](const std::vector<Value> &arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0].data))
      throw KymaError({"fileExists expects one path", {1, 1}, false, "K5201"});
    std::string error;
    const bool found = capabilities.files->exists(std::get<std::string>(arguments[0].data), error);
    if (!error.empty())
      throw KymaError({std::move(error), {1, 1}, false, "K5201"});
    return Value(found);
  };
  global->define("fileExists", Value(fileExists), false);

  auto removePath = std::make_shared<Function>();
  removePath->native = true;
  removePath->nativeCall = [capabilities](const std::vector<Value> &arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0].data))
      throw KymaError(
          {"removePath expects one file or empty-directory path", {1, 1}, false, "K5202"});
    std::string error;
    const bool removed =
        capabilities.files->remove(std::get<std::string>(arguments[0].data), error);
    if (!error.empty())
      throw KymaError({std::move(error), {1, 1}, false, "K5202"});
    return Value(removed);
  };
  global->define("removePath", Value(removePath), false);

  auto listDirectory = std::make_shared<Function>();
  listDirectory->native = true;
  listDirectory->nativeCall = [&interpreter, capabilities](const std::vector<Value> &arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0].data))
      throw KymaError({"listDirectory expects one directory path", {1, 1}, false, "K5203"});
    std::string error;
    auto names = capabilities.files->list(std::get<std::string>(arguments[0].data), error);
    if (!names)
      throw KymaError({std::move(error), {1, 1}, false, "K5203"});
    auto result = interpreter.heap().allocateArray();
    for (auto &name : *names)
      result->elements.emplace_back(std::move(name));
    return Value(result);
  };
  global->define("listDirectory", Value(listDirectory), false);

  auto readJsonFile = std::make_shared<Function>();
  readJsonFile->native = true;
  readJsonFile->nativeCall = [&interpreter, capabilities](const std::vector<Value> &arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0].data))
      throw KymaError({"readJsonFile expects one path", {1, 1}, false, "K5204"});
    std::string error;
    auto contents = capabilities.files->read(std::get<std::string>(arguments[0].data), error);
    if (!contents)
      throw KymaError({std::move(error), {1, 1}, false, "K5204"});
    return parseJsonValue(*contents, interpreter);
  };
  global->define("readJsonFile", Value(readJsonFile), false);

  auto writeJsonFile = std::make_shared<Function>();
  writeJsonFile->native = true;
  writeJsonFile->nativeCall = [capabilities](const std::vector<Value> &arguments) {
    if (arguments.size() != 2 || !std::holds_alternative<std::string>(arguments[0].data))
      throw KymaError({"writeJsonFile expects a path and value", {1, 1}, false, "K5205"});
    std::string error;
    if (!capabilities.files->write(std::get<std::string>(arguments[0].data),
                                   stringifyJsonValue(arguments[1]), error))
      throw KymaError({std::move(error), {1, 1}, false, "K5205"});
    return Value(true);
  };
  global->define("writeJsonFile", Value(writeJsonFile), false);

  auto fileSystem = interpreter.heap().allocate();
  fileSystem->fields["read"] = Value(read);
  fileSystem->fields["write"] = Value(write);
  fileSystem->fields["readJson"] = Value(readJsonFile);
  fileSystem->fields["writeJson"] = Value(writeJsonFile);
  fileSystem->fields["createDirectory"] = Value(createDirectory);
  fileSystem->fields["exists"] = Value(fileExists);
  fileSystem->fields["remove"] = Value(removePath);
  fileSystem->fields["list"] = Value(listDirectory);
  global->define("fs", Value(fileSystem), false);

  auto run = std::make_shared<Function>();
  run->native = true;
  run->nativeCall = [capabilities](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<std::string>(a[0].data))
      throw KymaError({"processRun expects a shell command string", {1, 1}, false});
    return Value(
        static_cast<int64_t>(capabilities.processes->run(std::get<std::string>(a[0].data))));
  };
  global->define("processRun", Value(run), false);
  global->define("build", Value(run), false);
  auto environment = std::make_shared<Function>();
  environment->native = true;
  environment->nativeCall = [capabilities](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<std::string>(a[0].data))
      throw KymaError({"processEnv expects a variable name", {1, 1}, false});
    auto value = capabilities.processes->environment(std::get<std::string>(a[0].data));
    return value ? Value(std::move(*value)) : Value();
  };
  global->define("processEnv", Value(environment), false);
  auto sleep = std::make_shared<Function>();
  sleep->native = true;
  sleep->nativeCall = [capabilities](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<int64_t>(a[0].data))
      throw KymaError({"sleep expects milliseconds", {1, 1}, false});
    capabilities.clock->sleep(std::chrono::milliseconds(std::get<int64_t>(a[0].data)));
    return Value();
  };
  global->define("sleep", Value(sleep), false);
  global->define("wait", Value(sleep), false);
  auto get = std::make_shared<Function>();
  get->native = true;
  get->nativeCall = [capabilities](const std::vector<Value> &a) {
    if (a.size() != 1 || !std::holds_alternative<std::string>(a[0].data))
      throw KymaError({"httpGet expects a URL string", {1, 1}, false});
    std::string error;
    auto response = capabilities.network->get(std::get<std::string>(a[0].data), error);
    if (!response)
      throw KymaError(
          {"GET " + std::get<std::string>(a[0].data) + ": " + std::move(error), {1, 1}, false});
    return Value(std::move(*response));
  };
  global->define("httpGet", Value(get), false);

  auto jsonParse = std::make_shared<Function>();
  jsonParse->native = true;
  jsonParse->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0].data))
      throw KymaError({"jsonParse expects one JSON string", {1, 1}, false, "K5100"});
    return parseJsonValue(std::get<std::string>(arguments[0].data), interpreter);
  };
  global->define("jsonParse", Value(jsonParse), false);

  auto jsonStringify = std::make_shared<Function>();
  jsonStringify->native = true;
  jsonStringify->nativeCall = [](const std::vector<Value> &arguments) {
    if (arguments.size() != 1)
      throw KymaError({"jsonStringify expects one value", {1, 1}, false, "K5101"});
    return Value(stringifyJsonValue(arguments[0]));
  };
  global->define("jsonStringify", Value(jsonStringify), false);

  auto fetch = std::make_shared<Function>();
  fetch->native = true;
  fetch->nativeCall = [&interpreter, capabilities](const std::vector<Value> &arguments) {
    if (arguments.empty() || arguments.size() > 2 ||
        !std::holds_alternative<std::string>(arguments[0].data) ||
        (arguments.size() == 2 && !std::holds_alternative<ObjectPtr>(arguments[1].data)))
      throw KymaError({"fetch expects a URL and optional request-options object", {1, 1}, false});
    const auto url = std::get<std::string>(arguments[0].data);
    std::string method = "GET";
    std::optional<std::string> requestBody;
    if (arguments.size() == 2) {
      const auto options = std::get<ObjectPtr>(arguments[1].data);
      if (const auto found = options->fields.find("method"); found != options->fields.end()) {
        if (!std::holds_alternative<std::string>(found->second.data))
          throw KymaError({"fetch option 'method' must be a string", {1, 1}, false});
        method = std::get<std::string>(found->second.data);
        std::transform(method.begin(), method.end(), method.begin(),
                       [](unsigned char character) { return std::toupper(character); });
      }
      if (const auto found = options->fields.find("body"); found != options->fields.end()) {
        if (!std::holds_alternative<std::string>(found->second.data))
          throw KymaError({"fetch option 'body' must be a JSON string", {1, 1}, false});
        requestBody = std::get<std::string>(found->second.data);
      }
    }
    std::string error;
    auto body = capabilities.network->request(method, url, requestBody, error);
    if (!body)
      throw KymaError({method + " " + url + ": " + std::move(error), {1, 1}, false});
    auto response = interpreter.heap().allocate();
    response->fields["ok"] = Value(true);
    response->fields["status"] = Value(std::int64_t{200});
    response->fields["url"] = Value(url);
    response->fields["method"] = Value(method);
    auto textMethod = std::make_shared<Function>();
    textMethod->native = true;
    textMethod->nativeCall = [contents = *body](const std::vector<Value> &methodArguments) {
      if (!methodArguments.empty())
        throw KymaError({"response.text expects no arguments", {1, 1}, false});
      return Value(contents);
    };
    response->fields["text"] = Value(textMethod);
    auto jsonMethod = std::make_shared<Function>();
    jsonMethod->native = true;
    jsonMethod->nativeCall =
        [&interpreter, contents = std::move(*body)](const std::vector<Value> &methodArguments) {
          if (!methodArguments.empty())
            throw KymaError({"response.json expects no arguments", {1, 1}, false});
          return parseJsonValue(contents, interpreter);
        };
    response->fields["json"] = Value(jsonMethod);
    return Value(response);
  };
  global->define("fetch", Value(fetch), false);

  auto filter = std::make_shared<Function>();
  filter->native = true;
  filter->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    if (arguments.size() != 2 || !std::holds_alternative<ArrayPtr>(arguments[0].data) ||
        !std::holds_alternative<FunctionPtr>(arguments[1].data))
      throw KymaError({"filter expects an array and predicate function", {1, 1}, false});
    auto output = interpreter.heap().allocateArray();
    const auto input = std::get<ArrayPtr>(arguments[0].data);
    const auto predicate = std::get<FunctionPtr>(arguments[1].data);
    for (std::size_t index = 0; index < input->elements.size(); ++index) {
      std::vector<Value> predicateArguments{input->elements[index]};
      if (!predicate->native && predicate->declaration.params.size() > 1)
        predicateArguments.emplace_back(static_cast<std::int64_t>(index));
      if (predicate->call(predicateArguments, interpreter).isTruthy())
        output->elements.push_back(input->elements[index]);
    }
    return Value(output);
  };
  global->define("filter", Value(filter), false);

  auto bubbleSort = std::make_shared<Function>();
  bubbleSort->native = true;
  bubbleSort->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    if (arguments.empty() || arguments.size() > 2 ||
        !std::holds_alternative<ArrayPtr>(arguments[0].data) ||
        (arguments.size() == 2 && !std::holds_alternative<FunctionPtr>(arguments[1].data)))
      throw KymaError({"bubbleSort expects an array and optional comparator", {1, 1}, false});
    auto output = interpreter.heap().allocateArray();
    output->elements = std::get<ArrayPtr>(arguments[0].data)->elements;
    const auto comparator =
        arguments.size() == 2 ? std::get<FunctionPtr>(arguments[1].data) : FunctionPtr{};
    const auto shouldSwap = [&](const Value &left, const Value &right) {
      if (comparator)
        return comparator->call({left, right}, interpreter).isTruthy();
      if (std::holds_alternative<std::int64_t>(left.data) &&
          std::holds_alternative<std::int64_t>(right.data))
        return std::get<std::int64_t>(left.data) > std::get<std::int64_t>(right.data);
      if ((std::holds_alternative<std::int64_t>(left.data) ||
           std::holds_alternative<double>(left.data)) &&
          (std::holds_alternative<std::int64_t>(right.data) ||
           std::holds_alternative<double>(right.data))) {
        const auto number = [](const Value &value) {
          return std::holds_alternative<std::int64_t>(value.data)
                     ? static_cast<double>(std::get<std::int64_t>(value.data))
                     : std::get<double>(value.data);
        };
        return number(left) > number(right);
      }
      if (std::holds_alternative<std::string>(left.data) &&
          std::holds_alternative<std::string>(right.data))
        return std::get<std::string>(left.data) > std::get<std::string>(right.data);
      throw KymaError({"default bubbleSort supports only numbers or strings", {1, 1}, false});
    };
    for (std::size_t remaining = output->elements.size(); remaining > 1; --remaining) {
      bool changed = false;
      for (std::size_t index = 1; index < remaining; ++index) {
        if (!shouldSwap(output->elements[index - 1], output->elements[index]))
          continue;
        std::swap(output->elements[index - 1], output->elements[index]);
        changed = true;
      }
      if (!changed)
        break;
    }
    return Value(output);
  };
  global->define("bubbleSort", Value(bubbleSort), false);
  global->define("sort", Value(bubbleSort), false);

  auto call = std::make_shared<Function>();
  call->native = true;
  call->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    if (arguments.empty() || arguments.size() > 2 ||
        !std::holds_alternative<FunctionPtr>(arguments[0].data) ||
        (arguments.size() == 2 && !std::holds_alternative<ArrayPtr>(arguments[1].data)))
      throw KymaError({"call expects a function and optional argument array", {1, 1}, false});
    std::vector<Value> invocationArguments;
    if (arguments.size() == 2)
      invocationArguments = std::get<ArrayPtr>(arguments[1].data)->elements;
    return std::get<FunctionPtr>(arguments[0].data)->call(invocationArguments, interpreter);
  };
  global->define("call", Value(call), false);

  auto process = interpreter.heap().allocate();
  process->fields["json"] = Value(jsonParse);
  process->fields["stringify"] = Value(jsonStringify);
  process->fields["run"] = Value(run);
  process->fields["env"] = Value(environment);
  global->define("process", Value(process), false);

  auto createApiStore = std::make_shared<Function>();
  createApiStore->native = true;
  createApiStore->nativeCall = [&interpreter](const std::vector<Value> &arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<ArrayPtr>(arguments[0].data))
      throw KymaError({"createApiStore expects an initial record array", {1, 1}, false});
    auto store = interpreter.heap().allocate();
    store->fields["records"] = arguments[0];
    const auto records = std::get<ArrayPtr>(arguments[0].data);

    auto list = std::make_shared<Function>();
    list->native = true;
    list->nativeCall = [records](const std::vector<Value> &methodArguments) {
      if (!methodArguments.empty())
        throw KymaError({"store.list expects no arguments", {1, 1}, false});
      return Value(records);
    };
    store->fields["list"] = Value(list);

    auto getRecord = std::make_shared<Function>();
    getRecord->native = true;
    getRecord->nativeCall = [records](const std::vector<Value> &methodArguments) {
      if (methodArguments.size() != 1)
        throw KymaError({"store.get expects an id", {1, 1}, false});
      for (const auto &record : records->elements)
        if (const auto object = std::get_if<ObjectPtr>(&record.data); object && *object) {
          const auto id = (*object)->fields.find("id");
          if (id != (*object)->fields.end() && id->second.equals(methodArguments[0]))
            return record;
        }
      return Value();
    };
    store->fields["get"] = Value(getRecord);

    auto createRecord = std::make_shared<Function>();
    createRecord->native = true;
    createRecord->nativeCall = [records](const std::vector<Value> &methodArguments) {
      if (methodArguments.size() != 1 ||
          !std::holds_alternative<ObjectPtr>(methodArguments[0].data))
        throw KymaError({"store.create expects an object record", {1, 1}, false});
      records->elements.push_back(methodArguments[0]);
      return methodArguments[0];
    };
    store->fields["create"] = Value(createRecord);

    auto updateRecord = std::make_shared<Function>();
    updateRecord->native = true;
    updateRecord->nativeCall = [records](const std::vector<Value> &methodArguments) {
      if (methodArguments.size() != 2 ||
          !std::holds_alternative<ObjectPtr>(methodArguments[1].data))
        throw KymaError({"store.update expects an id and patch object", {1, 1}, false});
      for (auto &record : records->elements)
        if (const auto object = std::get_if<ObjectPtr>(&record.data); object && *object) {
          const auto id = (*object)->fields.find("id");
          if (id == (*object)->fields.end() || !id->second.equals(methodArguments[0]))
            continue;
          for (const auto &[name, value] : std::get<ObjectPtr>(methodArguments[1].data)->fields)
            (*object)->fields.insert_or_assign(name, value);
          return record;
        }
      return Value();
    };
    store->fields["update"] = Value(updateRecord);

    auto removeRecord = std::make_shared<Function>();
    removeRecord->native = true;
    removeRecord->nativeCall = [records](const std::vector<Value> &methodArguments) {
      if (methodArguments.size() != 1)
        throw KymaError({"store.remove expects an id", {1, 1}, false});
      const auto before = records->elements.size();
      std::erase_if(records->elements, [&](const Value &record) {
        const auto object = std::get_if<ObjectPtr>(&record.data);
        if (!object || !*object)
          return false;
        const auto id = (*object)->fields.find("id");
        return id != (*object)->fields.end() && id->second.equals(methodArguments[0]);
      });
      return Value(records->elements.size() != before);
    };
    store->fields["remove"] = Value(removeRecord);
    return Value(store);
  };
  global->define("createApiStore", Value(createApiStore), false);
}
} // namespace kyma
