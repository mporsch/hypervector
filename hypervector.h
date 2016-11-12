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
  hypervector(
    typename std::enable_if<sizeof...(Args) <= N, size_t>::type dim1,
    Args&&... args) {
    assign(dim1, std::forward<Args>(args)...);
  }


  ~hypervector() {
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, void>::type
  resize(size_t dim1, Args&&... args) {
    resizeValue(dim1, std::forward<Args>(args)...);
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N - 1, void>::type
  resize(size_t dim1, Args&&... args) {
    resizeNoValue(dim1, std::forward<Args>(args)...);
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
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, void>::type
  resizeValue(size_t dim1, Args&&... args) {
    _dims[N - sizeof...(Args)] = dim1;
    resizeValue(std::forward<Args>(args)...);
  }

  void resizeValue(T&& val) {
    _vec.assign(size(), std::forward<T>(val));
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N - 1, void>::type
  resizeNoValue(size_t dim1, Args&&... args) {

    _dims[N - 1 - sizeof...(Args)] = dim1;

    resizeNoValue(std::forward<Args>(args)...);
  }

  void resizeNoValue(size_t dimN) {

    _dims[N - 1] = dimN;

    _vec.resize(size());
  }

private:
  size_t _dims[N];
  std::vector<T> _vec;
};

#endif // HYPERVECTOR_H

