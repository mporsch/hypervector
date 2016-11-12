#ifndef HYPERVECTOR_H
#define HYPERVECTOR_H

#include <cstddef>
#include <vector>

template<typename T, size_t N>
class hypervector
{
public:
  hypervector()
    : _dims{0} {
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) + 1 <= N, void>::type
  resize(size_t dim1, Args&&... args) {

    _dims[N - 1 - sizeof...(Args)] = dim1;

    resize(std::forward<Args>(args)...);
  }

  void resize(size_t dimN) {

    _dims[N - 1] = dimN;

    _vec.resize(size());
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, void>::type
  assign(size_t dim1, Args&&... args) {

    _dims[N - sizeof...(Args)] = dim1;

    assign(std::forward<Args>(args)...);
  }

  void assign(T&& val) {

    _vec.assign(size(), std::forward<T>(val));
  }


  size_t size() const {
    size_t wholeSize = 1;
    for (auto it = std::begin(_dims); it != std::end(_dims); ++it)
      wholeSize *= *it;
    return wholeSize;
  }

private:
  size_t _dims[N];
  std::vector<T> _vec;
};

#endif // HYPERVECTOR_H

