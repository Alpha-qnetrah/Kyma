#include "kyna/keywords.hpp"
#include "kyna/lexing/tokenizer.hpp"
#include <algorithm>
#include <cctype>

namespace kyna {
namespace {
class KynaTokenizer {
public:
  explicit KynaTokenizer(const SourceFile &input) : source(input) {}

  LexResult scan() {
    while (!atEnd()) {
      tokenStart = current;
      startLine = line;
      startColumn = column;
      try {
        scanToken();
      } catch (const KynaError &error) {
        result.diagnostics.push_back(error.diagnostic);
        result.tokens.push_back({TokenKind::Invalid,
                                 source.text.substr(tokenStart, current - tokenStart),
                                 currentSpan()});
      }
    }
    result.tokens.push_back(
        {TokenKind::End, "", makeSpan(current, current, line, column, line, column)});
    return std::move(result);
  }

private:
  const SourceFile &source;
  LexResult result;
  std::size_t tokenStart{0};
  std::size_t current{0};
  int line{1};
  int column{1};
  int startLine{1};
  int startColumn{1};

  [[nodiscard]] bool atEnd() const { return current >= source.text.size(); }
  char advance() {
    const char character = source.text[current++];
    if (character == '\n') {
      ++line;
      column = 1;
    } else {
      ++column;
    }
    return character;
  }
  [[nodiscard]] char peek() const { return atEnd() ? '\0' : source.text[current]; }
  [[nodiscard]] char peekNext() const {
    return current + 1 < source.text.size() ? source.text[current + 1] : '\0';
  }
  bool take(char expected) {
    if (peek() != expected)
      return false;
    advance();
    return true;
  }
  SourceSpan makeSpan(std::size_t start, std::size_t end, int firstLine, int firstColumn,
                      int lastLine, int lastColumn) const {
    return {source.id, start, end, firstLine, firstColumn, lastLine, lastColumn};
  }
  [[nodiscard]] SourceSpan currentSpan() const {
    return makeSpan(tokenStart, current, startLine, startColumn, line, column);
  }
  void add(TokenKind kind) {
    result.tokens.push_back(
        {kind, source.text.substr(tokenStart, current - tokenStart), currentSpan()});
  }
  [[noreturn]] void fail(std::string message, std::string code) const {
    Diagnostic diagnostic{std::move(message), currentSpan(), false};
    diagnostic.code = std::move(code);
    throw KynaError(diagnostic);
  }
  void scanString() {
    while (!atEnd() && peek() != '"') {
      if (peek() == '\n')
        fail("newline in string literal", "K1002");
      if (peek() == '\\' && peekNext() != '\0')
        advance();
      advance();
    }
    if (atEnd())
      fail("unterminated string literal", "K1001");
    advance();
    add(TokenKind::String);
  }
  void scanCharacter() {
    if (atEnd() || peek() == '\n')
      fail("unterminated character literal", "K1003");
    if (peek() == '\\')
      advance();
    if (atEnd())
      fail("unterminated character literal", "K1003");
    advance();
    if (!take('\''))
      fail("character literal must contain exactly one character", "K1004");
    add(TokenKind::Char);
  }
  void scanNumber() {
    while (std::isdigit(static_cast<unsigned char>(peek())))
      advance();
    bool floating = false;
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
      floating = true;
      advance();
      while (std::isdigit(static_cast<unsigned char>(peek())))
        advance();
    }
    add(floating ? TokenKind::Float : TokenKind::Int);
  }
  void scanIdentifier() {
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
      advance();
    add(keywordKind(source.text.substr(tokenStart, current - tokenStart)));
  }
  void scanBlockComment() {
    advance();
    while (!(peek() == '*' && peekNext() == '/')) {
      if (atEnd())
        fail("unterminated block comment", "K1005");
      advance();
    }
    advance();
    advance();
  }
  void scanToken() {
    const char character = advance();
    if (std::isspace(static_cast<unsigned char>(character)))
      return;
    if (character == '#') {
      while (!atEnd() && peek() != '\n')
        advance();
      return;
    }
    if (character == '/' && peek() == '/') {
      while (!atEnd() && peek() != '\n')
        advance();
      return;
    }
    if (character == '/' && peek() == '*') {
      scanBlockComment();
      return;
    }
    switch (character) {
    case '(':
      add(TokenKind::LeftParen);
      return;
    case ')':
      add(TokenKind::RightParen);
      return;
    case '{':
      add(TokenKind::LeftBrace);
      return;
    case '}':
      add(TokenKind::RightBrace);
      return;
    case '[':
      add(TokenKind::LeftBracket);
      return;
    case ']':
      add(TokenKind::RightBracket);
      return;
    case ',':
      add(TokenKind::Comma);
      return;
    case ':':
      add(TokenKind::Colon);
      return;
    case ';':
      add(TokenKind::Semicolon);
      return;
    case '.':
      add(TokenKind::Dot);
      return;
    case '+':
      add(TokenKind::Plus);
      return;
    case '*':
      add(TokenKind::Star);
      return;
    case '%':
      add(TokenKind::Percent);
      return;
    case '-':
      add(take('>') ? TokenKind::Arrow : TokenKind::Minus);
      return;
    case '=':
      add(take('=') ? TokenKind::EqualEqual : (take('>') ? TokenKind::FatArrow : TokenKind::Equal));
      return;
    case '!':
      add(take('=') ? TokenKind::BangEqual : TokenKind::Bang);
      return;
    case '<':
      add(take('=') ? TokenKind::LessEqual : TokenKind::Less);
      return;
    case '>':
      add(take('=') ? TokenKind::GreaterEqual : TokenKind::Greater);
      return;
    case '&':
      if (!take('&'))
        fail("expected '&' after '&'", "K1006");
      add(TokenKind::AndAnd);
      return;
    case '|':
      add(take('|') ? TokenKind::OrOr : TokenKind::Pipe);
      return;
    case '?':
      add(TokenKind::Question);
      return;
    case '/':
      add(TokenKind::Slash);
      return;
    case '"':
      scanString();
      return;
    case '\'':
      scanCharacter();
      return;
    default:
      if (std::isdigit(static_cast<unsigned char>(character)))
        scanNumber();
      else if (std::isalpha(static_cast<unsigned char>(character)) || character == '_')
        scanIdentifier();
      else
        fail("unexpected character: " + std::string(1, character), "K1000");
    }
  }
};
} // namespace

bool LexResult::ok() const {
  return std::none_of(diagnostics.begin(), diagnostics.end(),
                      [](const Diagnostic &diagnostic) { return !diagnostic.warning; });
}

LexResult tokenize(const SourceFile &source) { return KynaTokenizer(source).scan(); }

} // namespace kyna
