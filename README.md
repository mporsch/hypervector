# hypervector
A multi-dimensional vector based on a one-dimensional std::vector.

Intended to replace multi-dimensional arrays with the benefit of dynamic resizeability. While the number of dimensions is fixed at compile time, the size of the dimensions can be changed at runtime.
Compared to a multi-dimensional std::vector (`std::vector<std::vector<...<T>>>`) the hypervector will use the one reserved memory pool of its one-dimensional container without multiple allocations.

The number of dimensions is given as template parameter. The interface is designed to resemble std::vector, except that methods reqire a number of arguments according to the number of hypervector dimensions:
* `constructor()`
* `constructor(size_type dim1, size_type dim2, ... , const T& value)`
* `constructor(size_type dim1, size_type dim2, ... )`
* `assign(size_type dim1, size_type dim2, ... , const T& value)`
* `resize(size_type dim1, size_type dim2, ... )`
* `resize(size_type dim1, size_type dim2, ... , const T& value)`
* `at(size_type dim1, size_type dim2, ... )`
* `operator[](size_t)` returns views on subdimensions or slices that themselves can be sliced further up to individual storage elements
![slice](https://user-images.githubusercontent.com/1180665/87228520-0ee18380-c3a2-11ea-9fec-3d223672ae1a.png)

Implemented as C++11 variadic template. 

Build test using CMake or `$ g++ -o hypervector_test hypervector_test.cpp -std=c++11`
