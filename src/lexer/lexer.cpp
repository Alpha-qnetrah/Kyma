#include "kyma/lexer.hpp"
#include "kyma/keywords.hpp"
#include <cctype>

namespace kyma {
namespace {
class Scanner {
  const std::string &source;
  std::vector<Token> out;
  size_t start{0}, current{0};
  int line{1}, column{1}, startColumn{1};
  bool end() const { return current >= source.size(); }
  char advance() {
    char c = source[current++];
    if (c == '\n') {
      ++line;
      column = 1;
    } else
      ++column;
    return c;
  }
  char peek() const { return end() ? '\0' : source[current]; }
  char next() const { return current + 1 < source.size() ? source[current + 1] : '\0'; }
  bool take(char c) {
    if (peek() != c)
      return false;
    advance();
    return true;
  }
  void add(TokenKind k) {
    out.push_back({k, source.substr(start, current - start), {line, startColumn}});
  }
  [[noreturn]] void fail(const std::string &m) { throw KymaError({m, {line, startColumn}, false}); }
  void stringToken() {
    while (!end() && peek() != '"') {
      if (peek() == '\n')
        fail("newline in string literal");
      advance();
    }
    if (end())
      fail("unterminated string literal");
    advance();
    add(TokenKind::String);
  }
  void charToken() {
    if (end() || peek() == '\n')
      fail("unterminated character literal");
    if (peek() == '\\')
      advance();
    advance();
    if (!take('\''))
      fail("character literal must contain exactly one character");
    add(TokenKind::Char);
  }
  void number() {
    while (std::isdigit(static_cast<unsigned char>(peek())))
      advance();
    bool floating = false;
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(next()))) {
      floating = true;
      advance();
      while (std::isdigit(static_cast<unsigned char>(peek())))
        advance();
    }
    add(floating ? TokenKind::Float : TokenKind::Int);
  }
  void identifier() {
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
      advance();
    auto text = source.substr(start, current - start);
    add(keywordKind(text));
  }

public:
  explicit Scanner(const std::string &s) : source(s) {}
  std::vector<Token> scan() {
    while (!end()) {
      start = current;
      startColumn = column;
      char c = advance();
      if (std::isspace(static_cast<unsigned char>(c)))
        continue;
      if (c == '/' && peek() == '/') {
        while (!end() && peek() != '\n')
          advance();
        continue;
      }
      if (c == '/' && peek() == '*') {
        advance();
        while (!(peek() == '*' && next() == '/')) {
          if (end())
            fail("unterminated block comment");
          advance();
        }
        advance();
        advance();
        continue;
      }
      switch (c) {
      case '(':
        add(TokenKind::LeftParen);
        break;
      case ')':
        add(TokenKind::RightParen);
        break;
      case '{':
        add(TokenKind::LeftBrace);
        break;
      case '}':
        add(TokenKind::RightBrace);
        break;
      case '[':
        add(TokenKind::LeftBracket);
        break;
      case ']':
        add(TokenKind::RightBracket);
        break;
      case ',':
        add(TokenKind::Comma);
        break;
      case ':':
        add(TokenKind::Colon);
        break;
      case ';':
        add(TokenKind::Semicolon);
        break;
      case '.':
        add(TokenKind::Dot);
        break;
      case '+':
        add(TokenKind::Plus);
        break;
      case '*':
        add(TokenKind::Star);
        break;
      case '%':
        add(TokenKind::Percent);
        break;
      case '-':
        add(take('>') ? TokenKind::Arrow : TokenKind::Minus);
        break;
      case '=':
        add(take('=') ? TokenKind::EqualEqual
                      : (take('>') ? TokenKind::FatArrow : TokenKind::Equal));
        break;
      case '!':
        add(take('=') ? TokenKind::BangEqual : TokenKind::Bang);
        break;
      case '<':
        add(take('=') ? TokenKind::LessEqual : TokenKind::Less);
        break;
      case '>':
        add(take('=') ? TokenKind::GreaterEqual : TokenKind::Greater);
        break;
      case '&':
        if (take('&'))
          add(TokenKind::AndAnd);
        else
          fail("expected '&' after '&'");
        break;
      case '|':
        add(take('|') ? TokenKind::OrOr : TokenKind::Pipe);
        break;
      case '?':
        add(TokenKind::Question);
        break;
      case '/':
        add(TokenKind::Slash);
        break;
      case '"':
        stringToken();
        break;
      case '\'':
        charToken();
        break;
      default:
        if (std::isdigit(static_cast<unsigned char>(c)))
          number();
        else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
          identifier();
        else
          fail("unexpected character: " + std::string(1, c));
      }
    }
    out.push_back({TokenKind::End, "", {line, column}});
    return out;
  }
};
} // namespace
std::vector<Token> lex(const std::string &source) { return Scanner(source).scan(); }
} // namespace kyma
