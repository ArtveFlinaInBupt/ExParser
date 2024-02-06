#include "parser/grammar.h"

#include "util/all.h"

#include <algorithm>
#include <format>

namespace ep {

Symbol::Symbol(std::string v, Type type): v(std::move(v)), type(type) {}

bool Symbol::operator<(const Symbol &rhs) const {
  return v < rhs.v;
}

bool Symbol::operator==(const Symbol &rhs) const {
  return v == rhs.v;
}

Symbol Symbol::empty_symbol() {
  return {"", Symbol::Terminator};
}

std::string Symbol::to_string() const {
  if (v.empty())
    return "~";
  return v;
}

[[nodiscard]] std::string
to_string(const std::pair<Symbol, std::vector<Symbol>> &production) {
  const auto &[lhs, rhs] = production;
  std::string buf = std::format("{} -> ", lhs.to_string());
  for (const auto &symbol : rhs)
    buf.append(symbol.to_string()).append(1, ' ');
  buf.pop_back();
  return buf;
}

std::string to_string(
    const std::map<Symbol, std::set<Symbol>> &set, const std::string &name
) {
  std::string buf;
  for (const auto &[symbol, symbol_set] : set) {
    buf.append(name).append("(").append(symbol.to_string()).append(") = {");
    for (const auto &symbol_ : symbol_set)
      buf.append(symbol_.to_string()).append(", ");
    if (!symbol_set.empty())
      buf.pop_back(), buf.pop_back();
    buf.append("}\n");
  }
  buf.pop_back();
  return buf;
}

std::string to_string(const PredictionTable &table) {
  std::set<Symbol> first_dimension{}, second_dimension{};
  std::map<Symbol, usize> max_column_len{};
  usize max_first_column_len = 0;

  for (const auto &[lhs, rhs_map] : table) {
    first_dimension.emplace(lhs);
    max_first_column_len =
        std::max(max_first_column_len, lhs.to_string().size());
    for (const auto &[rhs, production] : rhs_map) {
      second_dimension.emplace(rhs);
      max_column_len[rhs] =
          std::max(max_column_len[rhs], to_string({lhs, production}).size());
    }
  }

  for (auto &[_, len] : max_column_len)
    len = (len + 3) / 2 * 2;

  std::string buf;
  buf.append(max_first_column_len, ' ');
  buf.append(" | ");
  // clang-format off
  for (const auto &symbol : second_dimension)
    buf.append(
        std::format("{:^{}}", symbol.to_string(), max_column_len[symbol])
    ).append(" | ");
  // clang-format on
  buf.append(1, '\n');

  for (const auto &lhs : first_dimension) {
    buf.append(std::format("{:^{}}", lhs.to_string(), max_first_column_len))
        .append(" | ");
    auto row = table.find(lhs)->second;
    for (const auto &rhs : second_dimension) {
      if (auto it = row.find(rhs); it == row.end()) {
        buf.append(std::format("{:^{}}", "(nul)", max_column_len[rhs]));
        buf.append(" | ");
      } else {
        buf.append(std::format(
            "{:^{}}", to_string({lhs, it->second}), max_column_len[rhs]
        ));
        buf.append(" | ");
      }
    }
    buf.append(1, '\n');
  }

  return buf;
}

// Production::Production(Symbol lhs, std::vector<Symbol> rhs):
//     lhs(std::move(lhs)), rhs(std::move(rhs)) {}

// Production::Production(Symbol lhs, std::initializer_list<Symbol> rhs):
//     lhs(std::move(lhs)), rhs(rhs) {}

Grammar Grammar::from_str(const std::string &str) {
  Grammar grammar{};
  for (auto &&line : split(str, '\n')) {
    auto vec = split(line, " -> ");
    auto lhs = Symbol(vec[0], Symbol::NonTerminator);
    auto rhs_vec = std::vector<std::string>(split(vec[1], " | "));
    auto rhs_set = std::set<std::vector<Symbol>>{};
    for (auto &&rhs : rhs_vec) {
      auto rhs_symbol_vec = std::vector<Symbol>{};
      for (auto &&symbol : split(rhs, ' ')) {
        if (symbol == "ε")
          rhs_symbol_vec.emplace_back(Symbol::empty_symbol());
        else if (isupper(symbol[0]))
          rhs_symbol_vec.emplace_back(symbol, Symbol::NonTerminator);
        else
          rhs_symbol_vec.emplace_back(symbol, Symbol::Terminator);
      }
      rhs_set.emplace(std::move(rhs_symbol_vec));
    }
    grammar.push_productions(lhs, std::move(rhs_set));
  }
  return grammar;
}

Grammar Grammar::from_str(const std::string_view &sv) {
  return Grammar::from_str(std::string(sv));
}

std::string Grammar::to_string() const {
  auto terminators = get_terminators();
  auto nonterminators = get_nonterminators();

  std::string buf;

  buf.append("Terminators: {");
  for (const auto &terminator : terminators.first)
    buf.append(terminator.v).append(", ");
  if (!terminators.first.empty())
    buf.pop_back(), buf.pop_back();
  buf.append("}\n");

  buf.append("NonTerminators: {");
  for (const auto &nonterminator : nonterminators)
    buf.append(nonterminator.v).append(", ");
  if (!nonterminators.empty())
    buf.pop_back(), buf.pop_back();
  buf.append("}\n");

  buf.append("Productions: {\n");

  for (const auto &[lhs, rhs_set] : productions) {
    buf.append("  ").append(lhs.v).append(" -> ");
    for (const auto &rhs : rhs_set) {
      for (const auto &symbol : rhs) {
        buf.append(symbol.to_string()).append(1, ' ');
      }
      buf.append("| ");
    }
    buf.pop_back();
    buf.pop_back();
    buf.append(1, '\n');
  }
  buf.append("}");
  return buf;
}

std::pair<std::set<Symbol>, bool> Grammar::get_terminators() const {
  std::set<Symbol> terminators{};
  bool has_empty_symbol = false;
  for (const auto &[lhs, rhs_set] : productions)
    for (const auto &rhs : rhs_set)
      for (const auto &symbol : rhs) {
        if (symbol.v.empty())
          has_empty_symbol = true;
        else if (symbol.type == Symbol::Terminator)
          terminators.emplace(symbol);
      }
  return {terminators, has_empty_symbol};
}

std::set<Symbol> Grammar::get_nonterminators() const {
  std::set<Symbol> nonterminators{};
  for (const auto &[lhs, rhs_set] : productions) {
    nonterminators.emplace(lhs);
    for (const auto &rhs : rhs_set)
      for (const auto &symbol : rhs)
        if (symbol.type == Symbol::NonTerminator)
          nonterminators.emplace(symbol);
  }
  return nonterminators;
}

void Grammar::push_productions(
    const Symbol &lhs, const std::set<std::vector<Symbol>> &rhs_set
) {
  productions[lhs].insert(rhs_set.begin(), rhs_set.end());
}

void Grammar::push_productions(
    const Symbol &lhs, std::set<std::vector<Symbol>> &&rhs_set
) {
  productions[lhs].insert(rhs_set.begin(), rhs_set.end());
}

void Grammar::push_production(
    const Symbol &lhs, const std::vector<Symbol> &rhs
) {
  productions[lhs].emplace(rhs);
}

void Grammar::push_production(const Symbol &lhs, std::vector<Symbol> &&rhs) {
  productions[lhs].emplace(std::move(rhs));
}

inline bool is_left_recursive(const ProductionSet &prod_set) {
  const auto &[lhs, rhs_set] = prod_set;
  return std::any_of(rhs_set.begin(), rhs_set.end(), [&](const auto &rhs) {
    // std::cerr << rhs.front().v << ' ' << lhs.v << '\n';
    return rhs.front() == lhs;
  });
}

bool Grammar::is_left_recursive() const {
  return std::any_of(
      productions.begin(), productions.end(), ep::is_left_recursive
  );
}

void Grammar::eliminate_left_recursion() {
  Grammar new_grammar{};

  for (const auto &[lhs, rhs_set] : productions) {
    if (!ep::is_left_recursive({lhs, rhs_set})) {
      new_grammar.push_productions(lhs, rhs_set);
      continue;
    }

    Symbol new_lhs{lhs.v + "'", Symbol::NonTerminator};
    std::set<std::vector<Symbol>> new_rhs_set{};
    std::set<std::vector<Symbol>> old_rhs_set{};

    for (auto &&rhs : rhs_set) {
      if (rhs.front() == lhs) {
        auto new_rhs = rhs;
        new_rhs.erase(new_rhs.begin());
        new_rhs.emplace_back(new_lhs);
        new_rhs_set.emplace(std::move(new_rhs));
      } else {
        auto old_rhs = rhs;
        old_rhs.emplace_back(new_lhs);
        old_rhs_set.emplace(std::move(old_rhs));
      }
    }

    new_grammar.push_productions(lhs, std::move(old_rhs_set));
    new_grammar.push_productions(new_lhs, std::move(new_rhs_set));
    new_grammar.push_production(new_lhs, {Symbol::empty_symbol()});
  }

  *this = std::move(new_grammar);
}

inline bool is_left_factorable(const ProductionSet &prod_set) {
  const auto &[_, rhs_set] = prod_set;
  for (const auto &rhs1 : rhs_set) {
    if (!rhs1.empty())
      for (const auto &rhs2 : rhs_set) {
        if (!rhs2.empty())
          if (&rhs1 != &rhs2 && rhs1.front() == rhs2.front())
            return true;
      }
  }
  return false;
}

bool Grammar::is_left_factored() const {
  return std::any_of(
      productions.begin(), productions.end(), ep::is_left_factorable
  );
}

void Grammar::extract_left_factoring() {
  for (bool changed;;) {
    changed = false;

    Grammar new_grammar{};
    for (const auto &[lhs, rhs_set] : productions) {
      std::map<Symbol, std::set<std::vector<Symbol>>> new_rhs_set_map{};

      for (const auto &rhs : rhs_set) {
        if (rhs.empty())
          continue;

        auto new_rhs = rhs;
        new_rhs.erase(new_rhs.begin());
        new_rhs_set_map[rhs.front()].emplace(std::move(new_rhs));
      }

      int counter = 0;
      for (const auto &[symbol, new_rhs_set] : new_rhs_set_map) {
        if (new_rhs_set.size() > 1) {
          ++counter;
          // clang-format off
          Symbol new_lhs{
            lhs.v + std::to_string(counter), Symbol::NonTerminator
          };
          new_grammar.push_productions(lhs, {{symbol, new_lhs}});
          new_grammar.push_productions(new_lhs, new_rhs_set);
          // clang-format on
        } else {
          std::vector<Symbol> new_rhs{symbol};
          new_rhs.insert(
              new_rhs.end(), new_rhs_set.begin()->begin(),
              new_rhs_set.begin()->end()
          );
          new_grammar.push_production(lhs, std::move(new_rhs));
        }
      }
      changed |= counter > 0;
    }
    *this = std::move(new_grammar);

    if (!changed)
      break;
  }
}

FirstSet Grammar::build_first_set() const {
  FirstSet first_set{};

  auto terminators = get_terminators();

  for (const auto &terminator : terminators.first)
    first_set[terminator].emplace(terminator);
  if (terminators.second)
    first_set[Symbol::empty_symbol()].emplace(Symbol::empty_symbol());

  for (bool changed;;) {
    changed = false;

    for (const auto &[lhs, rhs_set] : productions) {
      for (const auto &rhs : rhs_set) {
        if (rhs.empty()) {
          changed |= first_set[lhs].emplace(Symbol::empty_symbol()).second;
          continue;
        }

        auto old_size = first_set[lhs].size();
        for (const auto &symbol : rhs) {
          auto &first_set_rhs = first_set[symbol];
          first_set[lhs].insert(first_set_rhs.begin(), first_set_rhs.end());
          if (!first_set_rhs.contains(Symbol::empty_symbol()))
            break;
        }
        changed |= first_set[lhs].size() != old_size;
      }
    }

    if (!changed)
      break;
  }

  return first_set;
}

FollowSet Grammar::build_follow_set(
    FirstSet &first_set, const Symbol &start_symbol
) const {
  FollowSet follow_set{};

  auto terminators = get_terminators();

  follow_set[start_symbol].emplace("$", Symbol::Terminator);

  for (bool changed;;) {
    changed = false;

    for (const auto &[lhs, rhs_set] : productions) {
      for (const auto &rhs : rhs_set) {
        for (auto it = rhs.begin(); it != rhs.end(); ++it) {
          if (it->type == Symbol::Terminator)
            continue;

          auto &follow_set_lhs = follow_set[*it];
          auto old_size = follow_set_lhs.size();

          auto it2 = std::next(it);
          if (it2 != rhs.end()) {
            auto &first_set_rhs = first_set[*it2];
            for (const auto &symbol : first_set_rhs)
              if (symbol != Symbol::empty_symbol())
                follow_set_lhs.emplace(symbol);
          }
          if (it2 == rhs.end() ||
              first_set[*it2].contains(Symbol::empty_symbol()))
            follow_set_lhs.insert(
                follow_set[lhs].begin(), follow_set[lhs].end()
            );

          changed |= follow_set_lhs.size() != old_size;
        }
      }
    }

    if (!changed)
      break;
  }

  return follow_set;
}

std::optional<std::string> Grammar::is_ll1(const Symbol &start_symbol) const {
  FirstSet first_set = build_first_set();
  FollowSet follow_set = build_follow_set(first_set, start_symbol);
  return is_ll1(first_set, follow_set);
}

std::optional<std::string>
Grammar::is_ll1(FirstSet &first_set, FollowSet &follow_set) const {
  for (const auto &[lhs, rhs_set] : productions) {
    for (const auto &rhs : rhs_set) {
      if (rhs.empty())
        continue;

      auto &first_set_rhs = first_set[rhs.front()];
      if (first_set_rhs.contains(Symbol::empty_symbol())) {
        auto &follow_set_lhs = follow_set[lhs];
        for (const auto &symbol : follow_set_lhs)
          if (first_set_rhs.contains(symbol)) {
            std::string buf = std::format(
                "FIRST({}) ∩ FOLLOW({}) = {{", rhs.front().v, lhs.v
            );
            for (const auto &symbol_ : first_set_rhs)
              buf.append(symbol_.to_string()).append(", ");
            buf.pop_back(), buf.pop_back();
            buf.append("}\n");
            return buf;
          }
      }
    }

    for (auto it1 = rhs_set.begin(); it1 != rhs_set.end(); ++it1) {
      for (auto it2 = std::next(it1); it2 != rhs_set.end(); ++it2) {
        auto &first_set_rhs1 = first_set[*it1->begin()];
        auto &first_set_rhs2 = first_set[*it2->begin()];

        std::set<Symbol> intersection{};
        std::set_intersection(
            first_set_rhs1.begin(), first_set_rhs1.end(),
            first_set_rhs2.begin(), first_set_rhs2.end(),
            std::inserter(intersection, intersection.begin())
        );
        if (!intersection.empty()) {
          std::string buf = std::format(
              "FIRST({}) ∩ FIRST({}) = {{", it1->begin()->v, it2->begin()->v
          );
          for (const auto &symbol : intersection)
            buf.append(symbol.to_string()).append(", ");
          buf.pop_back(), buf.pop_back();
          buf.append("}\n");
          return buf;
        }
      }
    }
  }
  return std::nullopt;
}

PredictionTable Grammar::build_prediction_table(const Symbol &start_symbol
) const {
  FirstSet first_set = build_first_set();
  FollowSet follow_set = build_follow_set(first_set, start_symbol);
  return build_prediction_table(first_set, follow_set);
}

PredictionTable Grammar::build_prediction_table(
    FirstSet &first_set, FollowSet &follow_set
) const {
  PredictionTable prediction_table{};

  for (const auto &[lhs, rhs_set] : productions) {
    for (const auto &rhs : rhs_set) {
      auto &first_set_rhs = first_set[rhs.front()];
      for (const auto &symbol : first_set_rhs)
        prediction_table[lhs][symbol] = rhs;

      if (first_set_rhs.contains(Symbol::empty_symbol()))
        for (const auto &symbol : follow_set[lhs])
          prediction_table[lhs][symbol] = rhs;

      if (rhs.empty()) {
        for (const auto &symbol : follow_set[lhs])
          prediction_table[lhs][symbol] = rhs;
        continue;
      }
    }
  }

  for (auto &[lhs, rhs_map] : prediction_table)
    for (auto it = rhs_map.begin(); it != rhs_map.end();)
      if (it->first == Symbol::empty_symbol())
        it = rhs_map.erase(it);
      else
        ++it;

  return prediction_table;
}

} // namespace ep
