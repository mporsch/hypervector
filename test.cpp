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

  hypervector<std::string, 2> hvec2(2, 6, "0");
  std::cout << hvec2 << std::endl;

  int i = 0;
  for (int x = 0; x < hvec2.size(0); ++x)
    for (int y = 0; y < hvec2.size(1); ++y)
      hvec2[x][y] = std::to_string(i++);
  std::cout << hvec2 << std::endl;
}
