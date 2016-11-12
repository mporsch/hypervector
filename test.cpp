#include <string>
#include <iostream>

#include "hypervector.h"

int main(int argc, char** argv) {

  hypervector<std::string, 3> hvec3;
  hvec3.resize(3, 3, 3);

  std::cout << "size " << hvec3.size() << std::endl;

  
  hvec3.assign(1, 1, 1, "hi");

  std::cout << "size " << hvec3.size() << std::endl;
}
