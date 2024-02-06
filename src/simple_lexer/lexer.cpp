#include "simple_lexer/lexer.h"

#include <cctype>
#include <set>

namespace ep {

Lexer::Lexer(std::string str): src_(std::move(str)) {}

std::optional<char> Lexer::peek(usize offset) const {
  if (pos_ + offset >= src_.size())
    return std::nullopt;
  return src_.at(pos_ + offset);
}

bool Lexer::reached_eof() const {
  return pos_ >= src_.size();
}

std::optional<char> Lexer::consume() {
  if (reached_eof())
    return std::nullopt;
  return src_.at(pos_++);
}

std::optional<Token> Lexer::next_token() {
  auto cur_char = consume();
  if (!cur_char)
    return std::nullopt;

  if (isspace(*cur_char))
    return consume_whitespace();
  if (isdigit(*cur_char))
    return consume_integer();
  return punctuator(*cur_char);
}

Token Lexer::consume_whitespace() {
  while (peek() && isspace(*peek()))
    consume();
  return Whitespace{};
}

Token Lexer::consume_integer() {
  while (peek() && isdigit(*peek()))
    consume();
  return Integer{};
}

Token Lexer::punctuator(char c) {
  static std::set<char> valid_punctuators{'(', ')', '+', '-', '*', '/'};
  if (valid_punctuators.contains(c))
    return Punctuator{c};
  return LexError{};
}

} // namespace ep
