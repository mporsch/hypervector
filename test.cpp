#include <string>
#include <iostream>

#include "hypervector.h"

int main(int argc, char** argv) {

  hypervector<std::string, 3> hvec3(1, 2, 3, "ho");

  std::cout << "size()=" << hvec3.size()
    << ", size(0)=" << hvec3.size(0)
    << ", size(1)=" << hvec3.size(1)
    << ", size(2)=" << hvec3.size(2) << std::endl;

  
  hvec3.assign(3, 2, 1, "hi");

  std::cout << "size()=" << hvec3.size()
    << ", size(0)=" << hvec3.size(0)
    << ", size(1)=" << hvec3.size(1)
    << ", size(2)=" << hvec3.size(2) << std::endl;

  hvec3.resize(1, 2, 3, "hu");

  std::cout << "size()=" << hvec3.size()
    << ", size(0)=" << hvec3.size(0)
    << ", size(1)=" << hvec3.size(1)
    << ", size(2)=" << hvec3.size(2) << std::endl;

  hvec3.resize(3, 2, 1);

  std::cout << "size()=" << hvec3.size()
    << ", size(0)=" << hvec3.size(0)
    << ", size(1)=" << hvec3.size(1)
    << ", size(2)=" << hvec3.size(2) << std::endl;

  std::cout << "at(2, 1, 0)=" << hvec3.at(2, 1, 0) << std::endl;

  hvec3.at(2, 1, 0) = "wow";

  std::cout << "at(2, 1, 0)=" << hvec3.at(2, 1, 0) << std::endl;
}
