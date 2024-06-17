#ifndef HYPERVECTOR_PRINT_H
#define HYPERVECTOR_PRINT_H

#include "hypervector_view.h"

#include <iostream>

template<typename T, size_t Dims, bool IsConst>
std::ostream& operator<<(
    std::ostream& os,
    const hypervector_view<T, Dims, IsConst>& hvec) {
  auto size = hvec.template sizeOf<0>();
  const char* separator = "";
  for(decltype(size) i = 0; i < size; ++i) {
    os << separator << "(" << hvec[i] << ")";
    separator = ", ";
  }
  return os;
}

template<typename T, bool IsConst>
std::ostream& operator<<(
    std::ostream& os,
    const hypervector_view<T, 1, IsConst>& hvec) {
  auto size = hvec.template sizeOf<0>();
  const char* separator = "";
  for(decltype(size) i = 0; i < size; ++i) {
    os << separator << hvec[i];
    separator = ", ";
  }
  return os;
}

#endif // HYPERVECTOR_PRINT_H
