#pragma once

#ifndef EP_PARSER_PARSER_H
#  define EP_PARSER_PARSER_H

#  include "parser/grammar.h"
#  include "simple_lexer/lexer.h"
#  include "util/all.h"

namespace ep {

class Parser {
  Lexer lexer_{};
  Grammar grammar_{};
  PredictionTable prediction_table_{};

  using OutputEntry = std::tuple<std::string, std::string, std::string>;

public:
  explicit Parser(Grammar grammar);

  void load_source(std::string src);

  [[nodiscard]] static std::vector<Symbol>
  convert_lexeme_to_symbol(const std::vector<Token> &token_stream);

  void parse_expression(std::vector<Symbol> &&symbol_stream);

  static std::string
  parse_procedure_to_string(std::vector<OutputEntry> &&output_buffer);
};

} // namespace ep

#endif // EP_PARSER_PARSER_H
