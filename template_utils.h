#pragma once 

#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <set>
#include <memory>
#include <stdexcept>

namespace Utils {
    // Extends `destination` by appending `source` to its end.
    template<typename T>
    void extendVector(std::vector<T>& destination, const std::vector<T>&& source) { destination.insert(destination.end(), source.begin(), source.end()); }

    // Lvalue version
    template<typename T>
    void extendVector(std::vector<T>& destination, const std::vector<T>& source) { destination.insert(destination.end(), source.begin(), source.end()); }
}

/**
 * Convert all std::strings to const char* using constexpr if (C++17)
 */
template<typename T>
auto convert(T&& t) {
  if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value) {
    return std::forward<T>(t).c_str();
  }
  else {
    return std::forward<T>(t);
  }
}

/**
 * printf like formatting for C++ with std::string
 * Original source: https://stackoverflow.com/a/26221725/11722
 */
template<typename ... Args>
std::string fmtInternal(const std::string& format, Args&& ... args)
{
  const auto size = snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args) ...) + 1;
  if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args ...);
  return std::string(buf.get(), buf.get() + size - 1);
}

template<typename ... Args>
std::string fmt(std::string fmt, Args&& ... args) {
  return fmtInternal(fmt, convert(std::forward<Args>(args))...);
}

// Prepends `"Error at line: {lineNo}. "` before the given error message.
template<typename ... Args>
std::string lineError(std::string fmt, int lineNo, Args&& ... args) {
  return fmtInternal("Error at line %d. ", lineNo) + fmtInternal(fmt, convert(std::forward<Args>(args))...);
}


template<typename Iterable>
using Iter = typename Iterable::iterator;

template<typename Iterable>
using IterableType = typename Iterable::value_type;

template <typename Iterable>
auto identity = [](IterableType<Iterable> iter) {return iter; };

// https://stackoverflow.com/a/20244786
template <typename Iterable>
std::string collectStrings(
    Iter<Iterable> begin,
    Iter<Iterable> end,
    std::function<std::string(IterableType<Iterable>)> extractor =
        identity<Iterable>,
    const std::string sep = ", "
) {
    int i = 0;
    std::stringstream s;
    for (auto iter = begin; iter != end; iter++, i++) {
        s << extractor(*iter);

        if (std::next(iter) != end) {
            s << sep;
        }
    }
    return s.str();
}

