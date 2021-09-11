#include "hypervector.h"

#include <iostream>
#include <string>

int main(int /*argc*/, char** /*argv*/) {
  bool success = true;

  { // test the different constructors, assign() and resize()
    {
      hypervector<std::string, 3> hvec;
      success &= (hvec.size() == 0);
      std::cout << "construct():\n" << hvec << "\n\n";
    }

    {
      hypervector<std::string, 3> hvec(1, 2, 3);
      success &= (hvec.size(0) == 1);
      success &= (hvec.size(1) == 2);
      success &= (hvec.size(2) == 3);
      success &= (hvec.size() == 1 * 2 * 3);
      std::cout << "construct(1, 2, 3):\n" << hvec << "\n\n";
    }

    hypervector<std::string, 3> hvec(1, 2, 3, "ho");
    success &= (hvec.size() == 1 * 2 * 3);
    std::cout << "construct(1, 2, 3, \"ho\"):\n" << hvec << "\n\n";

    hvec.assign(3, 2, 1, "hi");
    success &= (hvec.size() == 3 * 2 * 1);
    std::cout << "assign(3, 2, 1, \"hi\"):\n" << hvec << "\n\n";

    hvec.resize(1, 1, 1, "he");
    success &= (hvec.size() == 1 * 1 * 1);
    std::cout << "resize(1, 1, 1, \"he\"):\n" << hvec << "\n\n";

    hvec.resize(2, 2, 2, "ha");
    success &= (hvec.size() == 2 * 2 * 2);
    std::cout << "resize(2, 2, 2, \"ha\"):\n" << hvec << "\n\n";

    hvec.resize(3, 3, 3);
    success &= (hvec.size() == 3 * 3 * 3);
    std::cout << "resize(3, 3, 3):\n" << hvec << "\n\n";

    hvec.resize(3, 3, 0);
    success &= (hvec.size() == 3 * 3 * 0);
    std::cout << "resize(3, 3, 0):\n" << hvec << "\n\n";

    hvec.resize(3, 0, 3);
    success &= (hvec.size() == 3 * 0 * 3);
    std::cout << "resize(3, 0, 3):\n" << hvec << "\n\n";

    hvec.assign(0, 3, 3, "oh");
    success &= (hvec.size() == 0 * 3 * 3);
    std::cout << "assign(0, 3, 3, \"oh\"):\n" << hvec << "\n\n";
  }

  {
    int test[4][3][2];
    hypervector<int, 3> hvec(4, 3, 2);

    int i = 0;
    for (size_t x = 0; x < 4; ++x) {
      for (size_t y = 0; y < 3; ++y) {
        for (size_t z = 0; z < 2; ++z) {
          test[x][y][z] = i;
          hvec[x][y][z] = i++;

          // test operator[] and at()
          success &= (test[x][y][z] == hvec[x][y][z]);
          success &= (test[x][y][z] == hvec.at(x, y, z));
        }
      }
    }

    { // test begin() and end() globally
      auto firstArray = &test[0][0][0];
      const auto lastArray = firstArray + 4 * 3 * 2;
      auto firstHvec = std::begin(hvec);
      const auto lastHvec = std::end(hvec);
      for (; firstArray != lastArray && firstHvec != lastHvec; ++firstArray, ++firstHvec)
        success &= (*firstArray == *firstHvec);
    }

    { // test begin() and end() on (const) subdimension
      auto const subd = const_cast<const hypervector<int, 3>&>(hvec)[3];
      static_assert(std::is_convertible<decltype(hvec[3]), decltype(subd)>::value, "const type mismatch");
      auto firstArray = &test[3][0][0];
      const auto lastArray = firstArray + 3 * 2;
      auto firstHvec = std::begin(subd);
      const auto lastHvec = std::end(subd);
      for (; firstArray != lastArray && firstHvec != lastHvec; ++firstArray, ++firstHvec)
        success &= (*firstArray == *firstHvec);
    }
  }

  { // fool around with operator[] and at()
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

  return (success ? EXIT_SUCCESS : EXIT_FAILURE);
}
