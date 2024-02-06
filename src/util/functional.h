#pragma once

#ifndef EP_UTIL_FUNCTIONAL_H
#  define EP_UTIL_FUNCTIONAL_H

#  include "util/type.h"

#  include <string>
#  include <vector>

namespace ep {

inline std::vector<std::string> split(const std::string &str, char sep) {
  std::vector<std::string> result{};
  std::string::size_type pos = 0;
  while (pos < str.size()) {
    auto next_pos = str.find(sep, pos);
    if (next_pos == std::string::npos) {
      result.push_back(str.substr(pos));
      break;
    }
    result.push_back(str.substr(pos, next_pos - pos));
    pos = next_pos + 1;
  }
  return result;
}

inline std::vector<std::string> split(const std::string &str, const std::string &sep) {
  std::vector<std::string> result{};
  std::string::size_type pos = 0;
  while (pos < str.size()) {
    auto next_pos = str.find(sep, pos);
    if (next_pos == std::string::npos) {
      result.push_back(str.substr(pos));
      break;
    }
    result.push_back(str.substr(pos, next_pos - pos));
    pos = next_pos + sep.size();
  }
  return result;
}

} // namespace ep

#endif // EP_UTIL_FUNCTIONAL_H
