#pragma once

#ifndef EP_SIMPLE_LEXER_TOKEN_H
#  define EP_SIMPLE_LEXER_TOKEN_H

#  include <string_view>
#  include <variant>

namespace ep {

// enum class TokenBase : u8 {
//   Integer
//   Punctuator,
//   // Non-language tokens
//   Whitespace,
//   LexError,
// };

struct Integer {};

struct Punctuator {
  char punct{};

  explicit Punctuator(char punct): punct(punct) {}
};

struct Whitespace {};

struct LexError {};

using Token = std::variant<Integer, Punctuator, Whitespace, LexError>;

} // namespace ep

#endif // EP_SIMPLE_LEXER_TOKEN_H
