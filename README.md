# hypervector
A multi-dimensional container with an interface resembling `std::vector`.

## Motivation
Assume you intend to store multi-dimensional data, but the sizes are dynamic and the storage should adapt to that.
If in this case you have used a multi-dimensional `std::vector<std::vector<...<T>>>`, you may have realized its drawbacks:
cumbersome (re-)initialization of the components and non-ideal cache locality (since each of the MxNx[...] vectors allocates dynamic memory on its own).

## Design rationale
Intended to be used like static multi-dimensional arrays (`T[N][M]` or `std::array<std::array<T, M>, N>`) with the benefit of dynamic resizeability.

Compared with a multi-dimensional `std::vector`, the hypervector will use a single dynamic memory block without multiple scattered allocations.

The span/view data structure of the hypervector is designed such that one can simply 'shave off' the outer layers and get valid views on subdimensions.
I.e. each view points to the very same shared data; the only distinction is its lower dimensionality and extent.

The container data structure of the hypervector derives from the view. It extends its interface with size/shape modifiers and holds ownership of the pointed-to data while using the view's very own members.

## Interface
While the number of dimensions is fixed at compile time, the size of each dimension can be changed at runtime.
E.g. a 3D grid data storage can be resized in its width/height/depth according to changed parameters.

The number of dimensions is not limited; the hypervector can be templated as 1D array, 2D grid, 3D cuboid, 4D hypercube, etc.

The interface is designed to resemble `std::vector`, except that some methods require a number of arguments according to the number of dimensions the hypervector was templated with:
* `hypervector(size_type dim0, size_type dim1, ... , const T& value)`
* `hypervector(size_type dim0, size_type dim1, ... )`
* `hypervector(std::initializer_list<std::initializer_list<...>>)`
* `void assign(size_type dim0, size_type dim1, ... , const T& value)`
* `void resize(size_type dim0, size_type dim1, ... )`
* `void resize(size_type dim0, size_type dim1, ... , const T& value)`
* `void reserve(size_type dim0, size_type dim1, ... )`
* `reference at(size_type dim0, size_type dim1, ... )`
* `operator[](size_t)` returns views on subdimensions or slices that themselves can be sliced further up to individual storage elements
![slice](https://user-images.githubusercontent.com/1180665/87228520-0ee18380-c3a2-11ea-9fec-3d223672ae1a.png)
* ...

Implemented as C++11 variadic template.

## Build
Build test using CMake or `$ g++ -o hypervector_test hypervector_test.cpp -std=c++20`
