#pragma once

#ifndef EP_UTIL_OVERLOADED_H
#  define EP_UTIL_OVERLOADED_H

namespace ep {

template<class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace ep

#endif // EP_UTIL_OVERLOADED_H
