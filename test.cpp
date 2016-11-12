#include <string>
#include <iostream>

#include "hypervector.h"

std::ostream& operator<<(std::ostream &os, const hypervector<std::string, 3>& hvec3) {
  os << "size()=" << hvec3.size()
    << ", size(0)=" << hvec3.size(0)
    << ", size(1)=" << hvec3.size(1)
    << ", size(2)=" << hvec3.size(2) << std::endl;
  for (auto&&e : hvec3)
    os << e << std::endl;
  return os;
}

std::ostream& operator<<(std::ostream &os, const hypervector<std::string, 2>& hvec2) {
  os << "size()=" << hvec2.size()
    << ", size(0)=" << hvec2.size(0)
    << ", size(1)=" << hvec2.size(1) << std::endl;
  for (int x = 0; x < hvec2.size(0); ++x) {
    for (int y = 0; y < hvec2.size(1); ++y) {
      os << hvec2.at(x, y) << " ";
    }
    os << std::endl;
  }
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

  hypervector<std::string, 2> hvec2(3, 3, "0");
  std::cout << hvec2 << std::endl;

  int i = 0;
  for (int x = 0; x < hvec2.size(0); ++x)
    for (int y = 0; y < hvec2.size(1); ++y)
      hvec2.at(x, y) = std::to_string(i++);
  std::cout << hvec2 << std::endl;
}
