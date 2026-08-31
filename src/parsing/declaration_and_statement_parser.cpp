#include "kyna/parser.hpp"
#include <algorithm>
#include <sstream>
#include <type_traits>

namespace kyna {
Parser::Parser(std::vector<Token> t) : tokens(std::move(t)) {}
const Token &Parser::peek() const { return tokens[current]; }
const Token &Parser::previous() const { return tokens[current - 1]; }
bool Parser::check(TokenKind k) const { return peek().kind == k; }
bool Parser::match(TokenKind k) {
  if (!check(k))
    return false;
  ++current;
  return true;
}
const Token &Parser::consume(TokenKind k, const std::string &msg) {
  if (check(k))
    return tokens[current++];
  Diagnostic diagnostic{msg + ", got " + tokenName(peek().kind), peek().location, false};
  diagnostic.code = "K2000";
  if (peek().kind == TokenKind::End)
    incomplete = true;
  throw KynaError(diagnostic);
}
ExprPtr Parser::make(Expr::Node n, SourceLocation l) {
  return std::make_shared<Expr>(Expr{std::move(n), l});
}
StmtPtr Parser::make(Stmt::Node n, SourceLocation l) {
  return std::make_shared<Stmt>(Stmt{std::move(n), l});
}

std::vector<StmtPtr> Parser::parse() {
  auto result = parseRecovering(SourceFile{});
  if (!result.diagnostics.empty())
    throw KynaError(result.diagnostics.front());
  return std::move(result.tree.module.declarations);
}
ParseResult Parser::parseRecovering(const SourceFile &source) {
  ParsedModule module{source.id, source.path, {}, {}};
  diagnostics.clear();
  incomplete = false;
  seenNonImport = false;
  while (!check(TokenKind::End)) {
    const auto before = current;
    try {
      auto parsed = declaration();
      if (parsed) {
        std::visit(
            [&](const auto &node) {
              using T = std::decay_t<decltype(node)>;
              if constexpr (std::is_same_v<T, VarDecl> || std::is_same_v<T, FunctionDecl> ||
                            std::is_same_v<T, ClassDecl> || std::is_same_v<T, InterfaceDecl>) {
                if (node.exported)
                  module.exports.insert(node.name);
              }
            },
            parsed->node);
        module.declarations.push_back(std::move(parsed));
      }
    } catch (const KynaError &error) {
      diagnostics.push_back(error.diagnostic);
      synchronize();
    }
    if (current == before && !check(TokenKind::End))
      ++current;
  }
  return {SyntaxTree{std::move(module)}, std::move(diagnostics), incomplete};
}
void Parser::synchronize() {
  while (!check(TokenKind::End)) {
    if (current > 0 && previous().kind == TokenKind::Semicolon)
      return;
    switch (peek().kind) {
    case TokenKind::Import:
    case TokenKind::Export:
    case TokenKind::Let:
    case TokenKind::Set:
    case TokenKind::Func:
    case TokenKind::Class:
    case TokenKind::Intf:
    case TokenKind::If:
    case TokenKind::While:
    case TokenKind::Loop:
    case TokenKind::Return:
      return;
    case TokenKind::RightBrace:
      ++current;
      return;
    default:
      ++current;
      break;
    }
  }
}
std::vector<std::string> Parser::modifiers() {
  std::vector<std::string> r;
  while (check(TokenKind::Public) || check(TokenKind::Private) || check(TokenKind::Protected) ||
         check(TokenKind::Static) || check(TokenKind::Override) || check(TokenKind::Final) ||
         check(TokenKind::Abstract)) {
    r.push_back(peek().lexeme);
    ++current;
  }
  return r;
}
StmtPtr Parser::declaration() {
  const bool exported = match(TokenKind::Export);
  if (match(TokenKind::Import)) {
    if (exported)
      throw KynaError({"an import cannot be exported", previous().location, false, "K2010"});
    if (seenNonImport)
      throw KynaError(
          {"imports must precede other declarations", previous().location, false, "K2011"});
    --current;
    return importDeclaration();
  }
  seenNonImport = true;
  auto mods = modifiers();
  StmtPtr parsed;
  if (match(TokenKind::Func))
    parsed = functionDeclaration(std::move(mods));
  else if (match(TokenKind::Class))
    parsed = classDeclaration(std::move(mods));
  else if (match(TokenKind::Intf))
    parsed = interfaceDeclaration();
  else if (match(TokenKind::Let)) {
    --current;
    parsed = varDeclaration();
  } else if (match(TokenKind::Set)) {
    --current;
    parsed = varDeclaration();
  }
  if (parsed) {
    if (exported)
      markExported(parsed);
    return parsed;
  }
  if (exported)
    throw KynaError(
        {"export must precede a named declaration", previous().location, false, "K2012"});
  if (!mods.empty())
    throw KynaError(
        {"member modifier is only valid on a function or class", previous().location, false});
  return statement();
}
StmtPtr Parser::importDeclaration() {
  const Token start = consume(TokenKind::Import, "expected 'import'");
  const Token path = consume(TokenKind::String, "expected a quoted module path");
  consume(TokenKind::As, "expected 'as' after module path");
  const Token alias = consume(TokenKind::Identifier, "expected module alias");
  consume(TokenKind::Semicolon, "expected ';' after import");
  auto value =
      path.lexeme.size() >= 2 ? path.lexeme.substr(1, path.lexeme.size() - 2) : path.lexeme;
  return make(ImportDecl{std::move(value), alias.lexeme}, start.location);
}
void Parser::markExported(const StmtPtr &parsed) {
  std::visit(
      [](auto &node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, VarDecl> || std::is_same_v<T, FunctionDecl> ||
                      std::is_same_v<T, ClassDecl> || std::is_same_v<T, InterfaceDecl>)
          node.exported = true;
      },
      parsed->node);
}
TypeRef Parser::typeRef() {
  Token t = peek();
  if (!(check(TokenKind::Identifier) || check(TokenKind::IntType) || check(TokenKind::FloatType) ||
        check(TokenKind::NumType) || check(TokenKind::StrType) || check(TokenKind::CharType) ||
        check(TokenKind::BoolType) || check(TokenKind::Null) || check(TokenKind::VoidType) ||
        check(TokenKind::AnyType)))
    throw KynaError({"expected a type", peek().location, false});
  ++current;
  TypeRef r{t.lexeme, false, {}};
  if (match(TokenKind::Question))
    r.nullable = true;
  while (match(TokenKind::Pipe)) {
    TypeRef other = typeRef();
    r.unionTypes.push_back(std::move(other));
  }
  return r;
}
StmtPtr Parser::varDeclaration() {
  bool mut;
  Token t = peek();
  if (match(TokenKind::Let))
    mut = true;
  else {
    consume(TokenKind::Set, "expected 'let' or 'set'");
    mut = false;
  }
  Token name = consume(TokenKind::Identifier, "expected binding name");
  VarDecl d{mut, name.lexeme, {"void", false, {}}, false, nullptr};
  if (match(TokenKind::Colon)) {
    d.type = typeRef();
    d.hasType = true;
  }
  if (match(TokenKind::Equal))
    d.initializer = expression();
  else if (!d.hasType || d.type.name != "any")
    throw KynaError({"a non-any binding requires an initializer", name.location, false});
  consume(TokenKind::Semicolon, "expected ';' after declaration");
  return make(std::move(d), t.location);
}
StmtPtr Parser::functionDeclaration(std::vector<std::string> mods) {
  Token name = consume(TokenKind::Identifier, "expected function name");
  consume(TokenKind::LeftParen, "expected '(' after function name");
  std::vector<Param> params;
  if (!check(TokenKind::RightParen)) {
    do {
      Token p = consume(TokenKind::Identifier, "expected parameter name");
      consume(TokenKind::Colon, "function parameters require an explicit type");
      params.push_back({p.lexeme, typeRef()});
    } while (match(TokenKind::Comma));
  }
  consume(TokenKind::RightParen, "expected ')' after parameters");
  TypeRef ret{"void", false, {}};
  bool has = false;
  if (match(TokenKind::Colon)) {
    ret = typeRef();
    has = true;
  }
  auto body = block();
  return make(FunctionDecl{name.lexeme, std::move(params), std::move(ret), has, std::move(body),
                           std::move(mods)},
              name.location);
}
StmtPtr Parser::classDeclaration(std::vector<std::string> mods) {
  Token name = consume(TokenKind::Identifier, "expected class name");
  std::string parent;
  if (match(TokenKind::Extends))
    parent = consume(TokenKind::Identifier, "expected parent class name").lexeme;
  std::vector<std::string> interfaces;
  if (match(TokenKind::Implements)) {
    do {
      interfaces.push_back(consume(TokenKind::Identifier, "expected interface name").lexeme);
    } while (match(TokenKind::Comma));
  }
  consume(TokenKind::LeftBrace, "expected '{' after class header");
  ClassDecl c{name.lexeme, parent, {}, {}, std::move(mods), std::move(interfaces)};
  while (!check(TokenKind::RightBrace) && !check(TokenKind::End)) {
    auto mm = modifiers();
    if (match(TokenKind::Func) || match(TokenKind::Init)) {
      bool isInit = previous().kind == TokenKind::Init;
      Token n = isInit ? Token{TokenKind::Identifier, "init", previous().location}
                       : consume(TokenKind::Identifier, "expected method name");
      consume(TokenKind::LeftParen, "expected '(' after method name");
      std::vector<Param> ps;
      if (!check(TokenKind::RightParen)) {
        do {
          Token p = consume(TokenKind::Identifier, "expected parameter name");
          consume(TokenKind::Colon, "function parameters require an explicit type");
          ps.push_back({p.lexeme, typeRef()});
        } while (match(TokenKind::Comma));
      }
      consume(TokenKind::RightParen, "expected ')' after parameters");
      TypeRef rt{"void", false, {}};
      bool has = false;
      if (match(TokenKind::Colon)) {
        rt = typeRef();
        has = true;
      }
      StmtPtr body;
      if (std::find(mm.begin(), mm.end(), "abstract") != mm.end())
        consume(TokenKind::Semicolon, "expected ';' after abstract method signature");
      else
        body = block();
      c.methods.push_back(
          {n.lexeme, std::move(ps), std::move(rt), has, std::move(body), std::move(mm)});
      continue;
    }
    Token f = consume(TokenKind::Identifier, "expected field name or method");
    consume(TokenKind::Colon, "expected ':' after field name");
    auto ty = typeRef();
    ExprPtr init;
    if (match(TokenKind::Equal))
      init = expression();
    consume(TokenKind::Semicolon, "expected ';' after field");
    c.fields.push_back({f.lexeme, std::move(ty), std::move(init), std::move(mm)});
  }
  consume(TokenKind::RightBrace, "expected '}' after class");
  return make(std::move(c), name.location);
}
StmtPtr Parser::interfaceDeclaration() {
  Token n = consume(TokenKind::Identifier, "expected interface name");
  consume(TokenKind::LeftBrace, "expected '{' after interface name");
  InterfaceDecl i{n.lexeme, {}, {}};
  while (!check(TokenKind::RightBrace) && !check(TokenKind::End)) {
    Token x = consume(TokenKind::Identifier, "expected interface member");
    if (match(TokenKind::Colon)) {
      i.fields.push_back({x.lexeme, typeRef(), nullptr, {}});
      consume(TokenKind::Semicolon, "expected ';'");
    } else {
      consume(TokenKind::LeftParen, "expected '(' after method name");
      std::vector<Param> ps;
      if (!check(TokenKind::RightParen)) {
        do {
          Token p = consume(TokenKind::Identifier, "expected parameter");
          consume(TokenKind::Colon, "parameters require types");
          ps.push_back({p.lexeme, typeRef()});
        } while (match(TokenKind::Comma));
      }
      consume(TokenKind::RightParen, "expected ')'");
      consume(TokenKind::Colon, "interface methods require return types");
      auto rt = typeRef();
      consume(TokenKind::Semicolon, "expected ';'");
      i.methods.push_back({x.lexeme, std::move(ps), std::move(rt), true, nullptr, {}});
    }
  }
  consume(TokenKind::RightBrace, "expected '}' after interface");
  return make(std::move(i), n.location);
}

StmtPtr Parser::block() {
  Token t = consume(TokenKind::LeftBrace, "expected '{'");
  BlockStmt b;
  while (!check(TokenKind::RightBrace) && !check(TokenKind::End)) {
    if (check(TokenKind::Let) || check(TokenKind::Set))
      b.statements.push_back(varDeclaration());
    else if (check(TokenKind::Func)) {
      ++current;
      b.statements.push_back(functionDeclaration({}));
    } else if (check(TokenKind::Class)) {
      ++current;
      b.statements.push_back(classDeclaration({}));
    } else if (check(TokenKind::If) || check(TokenKind::While) || check(TokenKind::Loop) ||
               check(TokenKind::Break) || check(TokenKind::Continue) || check(TokenKind::Return) ||
               check(TokenKind::Try) || check(TokenKind::LeftBrace))
      b.statements.push_back(statement());
    else {
      auto e = expression();
      if (match(TokenKind::Semicolon))
        b.statements.push_back(make(ExprStmt{e}, e->location));
      else {
        if (!check(TokenKind::RightBrace))
          throw KynaError({"expected ';' after expression", peek().location, false});
        b.tail = e;
      }
    }
  }
  consume(TokenKind::RightBrace, "expected '}' after block");
  return make(std::move(b), t.location);
}
StmtPtr Parser::statement() {
  if (match(TokenKind::LeftBrace)) {
    --current;
    return block();
  }
  if (match(TokenKind::If)) {
    Token t = previous();
    consume(TokenKind::LeftParen, "expected '(' after if");
    auto c = expression();
    consume(TokenKind::RightParen, "expected ')' after condition");
    auto yes = block();
    StmtPtr no;
    if (match(TokenKind::Else))
      no = check(TokenKind::If) ? statement() : block();
    return make(IfStmt{c, yes, no}, t.location);
  }
  if (match(TokenKind::While)) {
    Token t = previous();
    consume(TokenKind::LeftParen, "expected '(' after while");
    auto c = expression();
    consume(TokenKind::RightParen, "expected ')' after condition");
    return make(WhileStmt{c, block(), ""}, t.location);
  }
  if (check(TokenKind::Identifier) && current + 1 < tokens.size() &&
      tokens[current + 1].kind == TokenKind::Colon) {
    std::string label = peek().lexeme;
    ++current;
    ++current;
    if (check(TokenKind::Loop)) {
      Token t = peek();
      ++current;
      StmtPtr init;
      ExprPtr cond, inc;
      if (match(TokenKind::LeftParen)) {
        if (!check(TokenKind::Semicolon)) {
          if (check(TokenKind::Let) || check(TokenKind::Set))
            init = varDeclaration();
          else {
            auto e = expression();
            consume(TokenKind::Semicolon, "expected ';' in loop");
            init = make(ExprStmt{e}, e->location);
          }
        } else
          ++current;
        if (!check(TokenKind::Semicolon))
          cond = expression();
        consume(TokenKind::Semicolon, "expected ';' in loop");
        if (!check(TokenKind::RightParen))
          inc = expression();
        consume(TokenKind::RightParen, "expected ')' after loop clauses");
      }
      return make(LoopStmt{init, cond, inc, block(), label}, t.location);
    }
    throw KynaError({"label must precede a loop", peek().location, false});
  }
  if (match(TokenKind::Loop)) {
    Token t = previous();
    StmtPtr init;
    ExprPtr cond, inc;
    if (match(TokenKind::LeftParen)) {
      if (!check(TokenKind::Semicolon)) {
        if (check(TokenKind::Let) || check(TokenKind::Set))
          init = varDeclaration();
        else {
          auto e = expression();
          consume(TokenKind::Semicolon, "expected ';' in loop");
          init = make(ExprStmt{e}, e->location);
        }
      } else
        ++current;
      if (!check(TokenKind::Semicolon))
        cond = expression();
      consume(TokenKind::Semicolon, "expected ';' in loop");
      if (!check(TokenKind::RightParen))
        inc = expression();
      consume(TokenKind::RightParen, "expected ')' after loop clauses");
    }
    return make(LoopStmt{init, cond, inc, block(), ""}, t.location);
  }
  if (match(TokenKind::Break)) {
    Token t = previous();
    std::string l;
    if (check(TokenKind::Identifier))
      l = peek().lexeme, ++current;
    consume(TokenKind::Semicolon, "expected ';' after break");
    return make(BreakStmt{l}, t.location);
  }
  if (match(TokenKind::Continue)) {
    Token t = previous();
    std::string l;
    if (check(TokenKind::Identifier))
      l = peek().lexeme, ++current;
    consume(TokenKind::Semicolon, "expected ';' after continue");
    return make(ContinueStmt{l}, t.location);
  }
  if (match(TokenKind::Try)) {
    Token t = previous();
    auto tryBranch = block();
    consume(TokenKind::Catch, "expected 'catch' after try block");
    consume(TokenKind::LeftParen, "expected '(' after catch");
    Token name = consume(TokenKind::Identifier, "expected catch binding name");
    consume(TokenKind::RightParen, "expected ')' after catch binding");
    return make(TryStmt{tryBranch, name.lexeme, block()}, t.location);
  }
  if (match(TokenKind::Return)) {
    Token t = previous();
    ExprPtr v;
    if (!check(TokenKind::Semicolon))
      v = expression();
    consume(TokenKind::Semicolon, "expected ';' after return");
    return make(ReturnStmt{v}, t.location);
  }
  auto e = expression();
  consume(TokenKind::Semicolon, "expected ';' after expression");
  return make(ExprStmt{e}, e->location);
}

} // namespace kyna
