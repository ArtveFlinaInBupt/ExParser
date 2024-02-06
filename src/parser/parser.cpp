#include "parser/parser.h"

#include <format>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>

using namespace std::string_literals;

namespace ep {

Parser::Parser(Grammar grammar): grammar_(std::move(grammar)) {
  std::cout << std::format(
                   "\033[32m-- Input grammar_ --\033[0m\n{}\n",
                   grammar_.to_string()
               )
            << std::endl;

  grammar_.eliminate_left_recursion();
  std::cout
      << std::format(
             "\033[32m-- Grammar eliminated left recursion --\033[0m\n{}\n",
             grammar_.to_string()
         )
      << std::endl;

  grammar_.extract_left_factoring();
  std::cout
      << std::format(
             "\033[32m-- Grammar extracted left factoring --\033[0m\n{}\n",
             grammar_.to_string()
         )
      << std::endl;

  auto first_set = grammar_.build_first_set();
  std::cout << std::format(
                   "\033[32m-- FIRST SET --\033[0m\n{}\n",
                   to_string(first_set, "FIRST")
               )
            << std::endl;

  auto follow_set =
      grammar_.build_follow_set(first_set, Symbol{"E", Symbol::NonTerminator});
  std::cout << std::format(
                   "\033[32m-- FOLLOW SET --\033[0m\n{}\n",
                   to_string(follow_set, "FOLLOW")
               )
            << std::endl;

  if (auto opt = grammar_.is_ll1(first_set, follow_set); opt) {
    std::cout << std::format(
                     "\033[32m-- Grammar is not LL(1) --\033[0m\n{}\n", *opt
                 )
              << std::endl;
    throw std::runtime_error("Grammar is not LL(1)");
  }
  std::cout << std::format(
                   "\033[32m-- Grammar is validated to be LL(1) --\033[0m\n"
               )
            << std::endl;

  prediction_table_ = grammar_.build_prediction_table(first_set, follow_set);
  std::cout << std::format(
                   "\033[32m-- Prediction table --\033[0m\n{}\n",
                   to_string(prediction_table_)
               )
            << std::endl;
}

void Parser::load_source(std::string src) {
  lexer_ = Lexer(std::move(src));

  std::vector<Token> token_stream;
  for (std::optional<Token> token; (token = lexer_.next_token());) {
    std::visit(
        overloaded{
            [](const LexError &) {
              throw std::runtime_error("Lex error");
            },
            [](const Whitespace &) {},
            [&](const auto &token) {
              token_stream.push_back(token);
            },
        },
        *token
    );
  }

  // std::cout << std::format("\033[32m-- Tokens --\033[0m\n");
  // for (const auto &token : token_stream) {
  //   std::visit(
  //       overloaded{
  //           [&](const Integer &) {
  //             std::cout << "Integer, ";
  //           },
  //           [&](const Punctuator &token) {
  //             std::cout << std::format("Punctuator(`{}`), ", token.punct);
  //           },
  //           [&](const auto &) {}},
  //       token
  //   );
  // }
  // std::cout << std::endl;

  parse_expression(convert_lexeme_to_symbol(token_stream));
}

std::vector<Symbol>
Parser::convert_lexeme_to_symbol(const std::vector<Token> &token_stream) {
  std::vector<Symbol> symbol_stream;
  for (const auto &token : token_stream) {
    std::visit(
        overloaded{
            [&](const Integer &) {
              symbol_stream.emplace_back(
                  "n", Symbol::Terminator
              ); // Change here
            },
            [&](const Punctuator &token) {
              symbol_stream.emplace_back(
                  std::string(1, token.punct), Symbol::Terminator
              );
            },
            [&](const auto &) {}},
        token
    );
  }
  return symbol_stream;
}

inline std::string seq_to_string(auto begin, auto end) {
  std::string buf;
  for (auto it = begin; it != end; ++it)
    buf += it->to_string();
  return buf;
}

inline std::string seq_to_string(const auto &seq) {
  return seq_to_string(seq.begin(), seq.end());
}

void Parser::parse_expression(std::vector<Symbol> &&symbol_stream) {
  symbol_stream.emplace_back("$", Symbol::Terminator);

  // std::cout << std::format("\033[32m-- Symbols --\033[0m\n");
  // for (const auto &symbol : symbol_stream)
  //   std::cout << std::format("{}, ", symbol.to_string());
  // std::cout << std::endl;

  bool has_error = false;
  std::vector<OutputEntry> output_buffer{
      {"<Stack>", "<Input>", "<Action>"}
  };

  std::vector<Symbol> stack;
  stack.emplace_back("$", Symbol::Terminator);
  stack.emplace_back("E", Symbol::NonTerminator);

  output_buffer.emplace_back(
      seq_to_string(stack), seq_to_string(symbol_stream), "Initial"
  );

  auto it = symbol_stream.begin();
  while (!stack.empty()) {
    auto top = stack.back();
    stack.pop_back();

    // if (it == symbol_stream.end()) {
    //   std::cout << std::format(
    //       "Not matched 3: {} != {}\n", top.to_string(),
    //       Symbol::empty_symbol().to_string()
    //   );
    //   throw std::runtime_error("Not matched");
    // }

    if (top.type == Symbol::Terminator) {
      if (top == Symbol::empty_symbol()) {
        output_buffer.emplace_back(
            seq_to_string(stack), seq_to_string(it, symbol_stream.end()), ""
        );
      } else if (top.v == it->v) {
        ++it;
        output_buffer.emplace_back(
            seq_to_string(stack), seq_to_string(it, symbol_stream.end()), ""
        );
      } else {
        has_error = true;
        output_buffer.emplace_back(
            "", "",
            std::format(
                "\033[31mError: {} not match {}\033[0m", top.to_string(),
                it->to_string()
            )
        );
      }
    } else {
      auto row = prediction_table_.at(top);
      for (; it != symbol_stream.end() && !row.count(*it); ++it) {
        has_error = true;
        output_buffer.emplace_back(
            "", "",
            std::format(
                "\033[31mError: {} not match {}\033[0m", top.to_string(),
                it->to_string()
            )
        );
      }
      if (it == symbol_stream.end()) {
        has_error = true;
        output_buffer.emplace_back(
            "", "",
            std::format(
                "\033[31mError: {} not match {}\033[0m", top.to_string(),
                Symbol::empty_symbol().to_string()
            )
        );
        break;
      }
      auto prediction = row.at(*it);
      stack.insert(stack.end(), prediction.rbegin(), prediction.rend());
      output_buffer.emplace_back(
          seq_to_string(stack), seq_to_string(it, symbol_stream.end()),
          to_string({top, prediction})
      );
    }
  }

  if (!has_error) {
    output_buffer.pop_back();
    std::get<2>(output_buffer.back()) = "\033[32mAccept\033[0m";
  }

  std::cout << std::format(
                   "\033[32m-- Parsing procedure --\033[0m\n{}\n",
                   parse_procedure_to_string(std::move(output_buffer))
               )
            << std::endl;
}

std::string
Parser::parse_procedure_to_string(std::vector<OutputEntry> &&output_buffer) {
  std::tuple<usize, usize, usize> max_len{0, 0, 0};
  for (const auto &[stack, input, action] : output_buffer) {
    max_len = std::make_tuple(
        std::max(std::get<0>(max_len), (stack.size() + 3) / 2 * 2),
        std::max(std::get<1>(max_len), (input.size() + 3) / 2 * 2),
        std::max(std::get<2>(max_len), (action.size() + 3) / 2 * 2)
    );
  }

  usize len_len = std::to_string(output_buffer.size() - 1).size();
  usize line_cnt = 0;

  std::string buf;
  for (const auto &[stack, input, action] : output_buffer) {
    if (action[0] == '<')
      buf += std::format(
          "\033[32m{:>{}} | {:^{}} | {:^{}} | {:^{}}\033[0m\n", "#", len_len,
          stack, std::get<0>(max_len), input, std::get<1>(max_len), action,
          std::get<2>(max_len)
      );
    else
      buf += std::format(
          "{:>{}} | {:<{}} | {:<{}} | {:<{}}\n", ++line_cnt, len_len, stack,
          std::get<0>(max_len), input, std::get<1>(max_len), action,
          std::get<2>(max_len)
      );
  }
  return buf;
}

} // namespace ep
