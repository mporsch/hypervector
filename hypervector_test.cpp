#include "hypervector.h"

#include <iostream>
#include <string>

int main(int /*argc*/, char** /*argv*/) {
  {
    hypervector<std::string, 3> hvec(1, 2, 3, "ho");
    std::cout << "construct(1, 2, 3, \"ho\"):\n" << hvec << "\n\n";

    hvec.assign(3, 2, 1, "hi");
    std::cout << "assign(3, 2, 1, \"hi\"):\n" << hvec << "\n\n";

    hvec.resize(1, 1, 1, "he");
    std::cout << "resize(1, 1, 1, \"he\"):\n" << hvec << "\n\n";

    hvec.resize(2, 2, 2, "ha");
    std::cout << "resize(2, 2, 2, \"ha\"):\n" << hvec << "\n\n";

    hvec.resize(3, 3, 3);
    std::cout << "resize(3, 3, 3):\n" << hvec << "\n\n";
  }

  {
    hypervector<std::string, 4> hvec(5, 4, 3, 2, "0");
    int i = 0;
    for (size_t w = 0; w < hvec.size(0); ++w)
      for (size_t x = 0; x < hvec.size(1); ++x)
        for (size_t y = 0; y < hvec.size(2); ++y)
          for (size_t z = 0; z < hvec.size(3); ++z)
            hvec.at(w, x, y, z) = std::to_string(i++);
    std::cout << "at(w, x, y, z):\n" << hvec << "\n\n";

    for (size_t w = 0; w < hvec.size(0); ++w)
        for (size_t x = 0; x < hvec.size(1); ++x)
          for (size_t y = 0; y < hvec.size(2); ++y)
            for (size_t z = 0; z < hvec.size(3); ++z)
              hvec[w][x][y][z] = std::to_string(--i);
    std::cout << "operator[w][x][y][z]:\n"  << hvec << "\n\n";
  }

  return EXIT_SUCCESS;
}
