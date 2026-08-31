#include "kyna/hir/hir_renderer.hpp"
#include "kyna/hir/syntax_lowering.hpp"
#include "kyna/lexing/tokenizer.hpp"
#include "kyna/parsing/module_parser.hpp"
#include "kyna/source/source_manager.hpp"
#include <cassert>

int main() {
  kyna::SourceManager sources;
  const auto source = sources.add("hir-test", "set left = 20; set right = 22; left + right;");
  auto lexed = kyna::tokenize(*sources.find(source));
  auto parsed = kyna::parseModule(*sources.find(source), std::move(lexed.tokens));
  assert(lexed.diagnostics.empty());
  assert(parsed.diagnostics.empty());

  auto lowered = kyna::lowerSyntaxToHir("hir-test", parsed.tree);
  assert(lowered.ok());
  assert(lowered.program->locals.size() == 2);
  assert(lowered.program->body.size() == 3);
  const auto listing = kyna::renderHir(*lowered.program);
  assert(listing.find("local %l0 left immutable") != std::string::npos);
  assert(listing.find("add") != std::string::npos);

  const auto functionSource = sources.add(
      "hir-functions",
      "func add(left: int, right: int): int { return left + right; } set answer = add(20, 22);");
  auto functionLexed = kyna::tokenize(*sources.find(functionSource));
  auto functionParsed =
      kyna::parseModule(*sources.find(functionSource), std::move(functionLexed.tokens));
  auto functionHir = kyna::lowerSyntaxToHir("hir-functions", functionParsed.tree);
  assert(functionHir.ok());
  assert(functionHir.program->functions.size() == 1);
  assert(functionHir.program->functions.front().parameters.size() == 2);
  const auto functionListing = kyna::renderHir(*functionHir.program);
  assert(functionListing.find("function @f0 add") != std::string::npos);
  assert(functionListing.find("call @f0") != std::string::npos);

  const auto logicalSource = sources.add("hir-logical", "set value = true && false || true;");
  auto logicalLexed = kyna::tokenize(*sources.find(logicalSource));
  auto logicalParsed =
      kyna::parseModule(*sources.find(logicalSource), std::move(logicalLexed.tokens));
  auto logicalHir = kyna::lowerSyntaxToHir("hir-logical", logicalParsed.tree);
  assert(logicalHir.ok());
  const auto logicalListing = kyna::renderHir(*logicalHir.program);
  assert(logicalListing.find("and") != std::string::npos);
  assert(logicalListing.find("or") != std::string::npos);

  const auto ifExpressionSource =
      sources.add("hir-if-expression", "set value = if (true) { 1 } else { 2 };");
  auto ifExpressionLexed = kyna::tokenize(*sources.find(ifExpressionSource));
  auto ifExpressionParsed = kyna::parseModule(*sources.find(ifExpressionSource),
                                               std::move(ifExpressionLexed.tokens));
  auto ifExpressionHir =
      kyna::lowerSyntaxToHir("hir-if-expression", ifExpressionParsed.tree);
  assert(ifExpressionHir.ok());
  assert(kyna::renderHir(*ifExpressionHir.program).find(" then ") != std::string::npos);

  const auto firstClassSource = sources.add(
      "hir-first-class",
      "func identity(value: int): int { return value; } set selected = identity; selected(1);");
  auto firstClassLexed = kyna::tokenize(*sources.find(firstClassSource));
  auto firstClassParsed =
      kyna::parseModule(*sources.find(firstClassSource), std::move(firstClassLexed.tokens));
  auto firstClassHir = kyna::lowerSyntaxToHir("hir-first-class", firstClassParsed.tree);
  assert(firstClassHir.ok());
  const auto firstClassListing = kyna::renderHir(*firstClassHir.program);
  assert(firstClassListing.find("function @f0") != std::string::npos);
  assert(firstClassListing.find("call.indirect") != std::string::npos);
}
