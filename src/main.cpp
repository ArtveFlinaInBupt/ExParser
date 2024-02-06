#include "parser/parser.h"

#include <iostream>
#include <random>

using namespace ep;
using namespace std::string_view_literals;

constexpr const auto grammar_sv = R"(E -> E + T | E - T | T
T -> T * F | T / F | F
F -> ( E ) | n)"sv; // Change here

int main() {
  auto parser = Parser(Grammar::from_str(grammar_sv));

  std::cerr << "Enter a line of expression, or 'q' to quit." << std::endl;
  for (std::string line; std::getline(std::cin, line);) {
    if (line[0] == 'q')
      break;
    try {
      parser.load_source(std::move(line));
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n' << std::endl;
      continue;
    }
  }

  return EXIT_SUCCESS;
}
