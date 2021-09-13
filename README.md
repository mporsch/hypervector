# hypervector
A multi-dimensional vector based on a one-dimensional std::vector.

Intended to be used like static multi-dimensional arrays (`T[N][M]` or `std::array<std::array<T, M>, N>`) with the benefit of dynamic resizeability.
While the number of dimensions is fixed at compile time, the size of each dimension can be changed at runtime.
E.g. a 3D grid data storage can be resized in its width/height/depth according to changed parameters.
The number of dimensions is not limited; the hypervector can be templated as 1D array, 2D grid, 3D cuboid, 4D hypercube, etc.
Compared to a multi-dimensional std::vector (`std::vector<std::vector<...<T>>>`) the hypervector will use the one (pre-reserved) memory of its one-dimensional container without multiple (per dimension) allocations.

The interface is designed to resemble std::vector, except that some methods require a number of arguments according to the number of dimensions the hypervector was templated with:
* `constructor()`
* `constructor(size_type dim0, size_type dim1, ... , const T& value)`
* `constructor(size_type dim0, size_type dim1, ... )`
* `void assign(size_type dim0, size_type dim1, ... , const T& value)`
* `void resize(size_type dim0, size_type dim1, ... )`
* `void resize(size_type dim0, size_type dim1, ... , const T& value)`
* `void reserve(size_type dim0, size_type dim1, ... )`
* `reference at(size_type dim0, size_type dim1, ... )`
* `operator[](size_t)` returns views on subdimensions or slices that themselves can be sliced further up to individual storage elements
![slice](https://user-images.githubusercontent.com/1180665/87228520-0ee18380-c3a2-11ea-9fec-3d223672ae1a.png)

Implemented as C++11 variadic template. 

Build test using CMake or `$ g++ -o hypervector_test hypervector_test.cpp -std=c++11`
