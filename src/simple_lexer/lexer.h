#pragma once

#ifndef EP_SIMPLE_LEXER_LEXER_H
#  define EP_SIMPLE_LEXER_LEXER_H

#  include "simple_lexer/token.h"
#  include "util/all.h"

#  include <optional>

namespace ep {

class Lexer {
  usize pos_{};
  std::string src_{};

public:
  Lexer() = default;

  explicit Lexer(std::string str);

  Lexer(const Lexer &rhs) = delete;

  Lexer(Lexer &&rhs) noexcept = default;

  Lexer &operator=(const Lexer &rhs) = delete;

  Lexer &operator=(Lexer &&rhs) noexcept  = default;

  [[nodiscard]] std::optional<Token> next_token();

  [[nodiscard]] bool reached_eof() const;

  [[nodiscard]] std::optional<char> peek(usize offset = 0) const;

  std::optional<char> consume();

  [[nodiscard]] Token consume_whitespace();

  [[nodiscard]] Token consume_integer();

  [[nodiscard]] static Token punctuator(char first_char);
};

} // namespace ep

#endif // EP_SIMPLE_LEXER_LEXER_H
