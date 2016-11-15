#ifndef HYPERVECTOR_H
#define HYPERVECTOR_H

#include <cstddef>
#include <vector>

template<typename T, size_t N>
class subdimension
{
  template<typename U, size_t M>
  friend class hypervector;

  using container = typename std::vector<T>;
  using reference = typename container::reference;
  using size_type = typename container::size_type;

  subdimension(T* data, const size_t* dims)
    : _data(data)
    , _dims(dims) {
  }

public:
  template<typename _T = T,
           typename = typename std::enable_if<(sizeof(_T), N > 1)>::type>
  subdimension<T, N - 1>
  operator[](size_type pos) {
    return subdimension<T, N - 1>(
      _data + pos * (_dims + N - 1),
      _dims);
  }

  template<typename _T = T,
           typename = typename std::enable_if<(sizeof(_T), N == 1)>::type>
  reference operator[](size_type pos) {
    return *(_data + pos);
  }

private:
  T* _data;
  const size_t* _dims;
};


template<typename T, size_t N>
class hypervector
{
public:
  using container       = typename std::vector<T>;
  using reference       = typename container::reference;
  using const_reference = typename container::const_reference;
  using size_type       = typename container::size_type;
  using iterator        = typename container::iterator;
  using const_iterator  = typename container::const_iterator;

public:
  hypervector()
    : _dims{0} {
  }


  // hypervector(size_type count, const T& value)
  template<typename ...Args>
  hypervector(
    typename std::enable_if<sizeof...(Args) == N, size_type>::type dim1,
    Args&&... args) {
    _assign(dim1, std::forward<Args>(args)...);
  }


  // hypervector(size_type count)
  template<typename ...Args>
  hypervector(
    typename std::enable_if<sizeof...(Args) == N - 1, size_type>::type dim1,
    Args&&... args) {
    _assign(dim1, std::forward<Args>(args)..., T());
  }


  ~hypervector() {
  }


  // resize(size_type count, const T& value)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, void>::type
  resize(size_type dim1, Args&&... args) {
    _resize(dim1, std::forward<Args>(args)...);
  }


  // resize(size_type count)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N - 1, void>::type
  resize(size_type dim1, Args&&... args) {
    _resize(dim1, std::forward<Args>(args)..., T());
  }


  // assign(size_type count, const T& value)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, void>::type
  assign(size_type dim1, Args&&... args) {
    _assign(dim1, std::forward<Args>(args)...);
  }


  // at(size_type pos)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N - 1, reference>::type
  at(size_type dim1, Args&&... args) {
    return _at(dim1, std::forward<Args>(args)...);
  }


  // at(size_type pos) const
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N - 1, const_reference>::type
  at(size_type dim1, Args&&... args) const {
    return _at(dim1, std::forward<Args>(args)...);
  }


  // operator[](size_type pos)
  template<typename _T = T,
           typename = typename std::enable_if<(sizeof(_T), N > 1)>::type>
  subdimension<T, N - 1>
  operator[](size_type pos) {
    return subdimension<T, N - 1>(
      &_vec[pos * _dims[N - 1]],
      &_dims[0]);
  }

  template<typename _T = T,
           typename = typename std::enable_if<(sizeof(_T), N == 1)>::type>
  reference operator[](size_type pos) {
    return _vec[pos];
  }


  size_type size() const {
    size_type wholeSize = 1;
    for (auto it = std::begin(_dims); it != std::end(_dims); ++it)
      wholeSize *= *it;
    return wholeSize;
  }


  size_type size(size_type dim) const {
    if (dim < N)
      return _dims[dim];
    else
      return 0;
  }


  iterator begin() {
    return _vec.begin();
  }


  iterator end() {
    return _vec.end();
  }


  const_iterator begin() const {
    return _vec.cbegin();
  }


  const_iterator end() const {
    return _vec.cend();
  }

private:
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, void>::type
  _resize(size_type dim1, Args&&... args) {
    _dims[N - sizeof...(Args)] = dim1;
    _resize(std::forward<Args>(args)...);
  }

  void _resize(T&& val) {
    _vec.resize(size(), std::forward<T>(val));
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, void>::type
  _assign(size_type dim1, Args&&... args) {

    _dims[N - sizeof...(Args)] = dim1;

    _assign(std::forward<Args>(args)...);
  }

  void _assign(T&& val) {
    _vec.assign(size(), std::forward<T>(val));
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N - 2, reference>::type
  _at(size_type dim1, size_type dim2, Args&&... args) {
    return _at(dim1 * _dims[sizeof...(Args) + 1] + dim2, std::forward<Args>(args)...);
  }

  reference _at(size_type dimN) {
    return _vec.at(dimN);
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N - 2, const_reference>::type
  _at(size_type dim1, size_type dim2, Args&&... args) const {
    return _at(dim1 * _dims[sizeof...(Args) + 1] + dim2, std::forward<Args>(args)...);
  }

  const_reference _at(size_type dimN) const {
    return _vec.at(dimN);
  }

private:
  size_type _dims[N];
  container _vec;
};

#endif // HYPERVECTOR_H

