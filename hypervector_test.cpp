#include <string>
#include <iostream>

#include "hypervector.h"

template<typename T, size_t N>
std::ostream& operator<<(std::ostream &os, const hypervector<T, N>& hvec) {
  std::cout << "dims = { ";
  for (size_t dim = 0; dim < N; ++dim)
    std::cout << hvec.size(dim) << " ";
  std::cout << "}\n";

  for (auto it = std::begin(hvec); it != std::end(hvec); ++it)
    os << *it << " ";

  return os;
}

int main(int argc, char** argv) {

  hypervector<std::string, 3> hvec3(1, 2, 3, "ho");
  std::cout << hvec3 << std::endl;

  hvec3.assign(3, 2, 1, "hi");
  std::cout << hvec3 << std::endl;

  hvec3.resize(1, 1, 1, "he");
  std::cout << hvec3 << std::endl;

  hvec3.resize(2, 2, 2, "ha");
  std::cout << hvec3 << std::endl;

  hvec3.resize(3, 3, 3);
  std::cout << hvec3 << std::endl;

  hypervector<std::string, 4> hvec4(5, 4, 3, 2, "0");
  int i = 0;
  for (size_t w = 0; w < hvec4.size(0); ++w)
    for (size_t x = 0; x < hvec4.size(1); ++x)
      for (size_t y = 0; y < hvec4.size(2); ++y)
        for (size_t z = 0; z < hvec4.size(3); ++z)
          hvec4.at(w, x, y, z) = std::to_string(i++);
  std::cout << hvec4 << "\n\n";

  for (size_t w = 0; w < hvec4.size(0); ++w)
      for (size_t x = 0; x < hvec4.size(1); ++x)
        for (size_t y = 0; y < hvec4.size(2); ++y)
          for (size_t z = 0; z < hvec4.size(3); ++z)
            hvec4[w][x][y][z] = std::to_string(--i);
  std::cout << hvec4 << "\n\n";
}
