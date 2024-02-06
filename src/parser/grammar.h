#pragma once

#ifndef EP_PARSER_GRAMMAR_H
#  define EP_PARSER_GRAMMAR_H

#endif // EP_PARSER_GRAMMAR_H

#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ep {

struct Symbol {
  std::string v{};

  enum Type { Terminator, NonTerminator } type;

  Symbol(const Symbol &rhs) = default;

  Symbol(Symbol &&rhs) noexcept = default;

  Symbol &operator=(const Symbol &rhs) = default;

  Symbol &operator=(Symbol &&rhs) noexcept = default;

  Symbol(std::string v, Type type);

  bool operator<(const Symbol &rhs) const;

  bool operator==(const Symbol &rhs) const;

  static Symbol empty_symbol();

  [[nodiscard]] std::string to_string() const;
};

using ProductionSet = std::pair<Symbol, std::set<std::vector<Symbol>>>;
using FirstSet = std::map<Symbol, std::set<Symbol>>;
using FollowSet = std::map<Symbol, std::set<Symbol>>;
using PredictionTable = std::map<Symbol, std::map<Symbol, std::vector<Symbol>>>;

[[nodiscard]] std::string
to_string(const std::pair<Symbol, std::vector<Symbol>> &production);

[[nodiscard]] std::string to_string(
    const std::map<Symbol, std::set<Symbol>> &set, const std::string &name
);

[[nodiscard]] std::string to_string(const PredictionTable &table);

struct Grammar {
  std::map<Symbol, std::set<std::vector<Symbol>>> productions{};

  Grammar() = default;

  Grammar(const Grammar &rhs) = delete;

  Grammar(Grammar &&rhs) noexcept = default;

  Grammar &operator=(Grammar &&rhs) noexcept = default;

  static Grammar from_str(const std::string &str);

  static Grammar from_str(const std::string_view &str);

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] std::pair<std::set<Symbol>, bool> get_terminators() const;

  [[nodiscard]] std::set<Symbol> get_nonterminators() const;

  void push_productions(
      const Symbol &lhs, const std::set<std::vector<Symbol>> &rhs_set
  );

  void
  push_productions(const Symbol &lhs, std::set<std::vector<Symbol>> &&rhs_set);

  void push_production(const Symbol &lhs, const std::vector<Symbol> &rhs);

  void push_production(const Symbol &lhs, std::vector<Symbol> &&rhs);

  [[nodiscard]] bool is_left_recursive() const;

  void eliminate_left_recursion();

  [[nodiscard]] bool is_left_factored() const;

  void extract_left_factoring();

  [[nodiscard]] FirstSet build_first_set() const;

  [[nodiscard]] FollowSet
  build_follow_set(FirstSet &first_set, const Symbol &start_symbol) const;

  [[nodiscard]] std::optional<std::string> is_ll1(const Symbol &start_symbol
  ) const;

  [[nodiscard]] std::optional<std::string>
  is_ll1(FirstSet &first_set, FollowSet &follow_set) const;

  [[nodiscard]] PredictionTable
  build_prediction_table(const Symbol &start_symbol) const;

  [[nodiscard]] PredictionTable
  build_prediction_table(FirstSet &first_set, FollowSet &follow_set) const;
};

} // namespace ep
