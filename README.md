# hypervector
A multi-dimensional vector based on a one-dimensional std::vector.

Intended to replace multi-dimensional arrays with the benefit of dynamic resizeability. Compared to a multi-dimensional std::vector the hypervector will use the one reserved memory pool of its one-dimensional std::vector without any reallocations.

The number of dimensions is passed as template parameter. The interface is designed to resemble std::vector, except that some methods reqire a number of arguments according to the number of hypervector dimensions.

Implemented as C++11 variadic template. 

Build test using `$ g++ -o test test.cpp -std=c++11`
