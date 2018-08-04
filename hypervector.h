#ifndef HYPERVECTOR_H
#define HYPERVECTOR_H

#include <cstddef>
#include <numeric>
#include <vector>

template<typename T, size_t N>
class hypervector;

namespace hypervector_detail {
  template<typename T, size_t N>
  class subdimension
  {
  public:
    using reference       = typename hypervector<T, N>::reference;
    using const_reference = typename hypervector<T, N>::const_reference;
    using size_type       = typename hypervector<T, N>::size_type;

  public:
    subdimension(const size_type* dims, const size_type* offsets, T* data)
      : _dims(dims)
      , _offsets(offsets)
      , _data(data) {
    }


    // operator[](size_type pos)
    template<typename _U = T,
             typename = typename std::enable_if<(sizeof(_U), N > 1)>::type>
    subdimension<T, N - 1>
    operator[](size_type pos) {
      return subdimension<T, N - 1>(
        _dims + 1,
        _offsets + 1,
        _data + pos * _offsets[0]);
    }

    template<typename _U = T,
             typename = typename std::enable_if<(sizeof(_U), N == 1)>::type>
    reference operator[](size_type pos) {
      return *(_data + pos);
    }


    // operator[](size_type pos) const
    template<typename _U = T,
             typename = typename std::enable_if<(sizeof(_U), N > 1)>::type>
    const subdimension<T, N - 1>
    operator[](size_type pos) const {
      return subdimension<T, N - 1>(
        _dims + 1,
        _offsets + 1,
        _data + pos * _offsets[0]);
    }

    template<typename _U = T,
             typename = typename std::enable_if<(sizeof(_U), N == 1)>::type>
    const_reference operator[](size_type pos) const {
      return *(_data + pos);
    }


    size_type size(size_type dim) const {
      if (dim < N)
        return _dims[dim];
      else
        return 0;
    }

  private:
    const size_type* _dims;
    const size_type* _offsets;
    T* _data;
  };
} // namespace hypervector_detail

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
    : _dims{0}
    , _offsets{0} {
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
  typename std::enable_if<sizeof...(Args) == N, reference>::type
  at(Args&&... args) {
    return _vec.at(_indexOf(std::forward<Args>(args)...));
  }


  // at(size_type pos) const
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, const_reference>::type
  at(Args&&... args) const {
    return _vec.at(_indexOf(std::forward<Args>(args)...));
  }


  // operator[](size_type pos)
  template<typename _T = T,
           typename = typename std::enable_if<(sizeof(_T), N > 1)>::type>
  hypervector_detail::subdimension<T, N - 1>
  operator[](size_type pos) {
    return hypervector_detail::subdimension<T, N - 1>(
      _dims + 1,
      _offsets + 1,
      _vec.data() + pos * _offsets[0]);
  }

  template<typename _T = T,
           typename = typename std::enable_if<(sizeof(_T), N == 1)>::type>
  reference operator[](size_type pos) {
    return _vec[pos];
  }


  // operator[](size_type pos) const
  template<typename _T = T,
           typename = typename std::enable_if<(sizeof(_T), N > 1)>::type>
  const hypervector_detail::subdimension<T, N - 1>
  operator[](size_type pos) const {
    return hypervector_detail::subdimension<T, N - 1>(
      _dims + 1,
      _offsets + 1,
      const_cast<T *>(_vec.data()) + pos * _offsets[0]);
  }

  template<typename _T = T,
           typename = typename std::enable_if<(sizeof(_T), N == 1)>::type>
  const_reference operator[](size_type pos) const {
    return _vec[pos];
  }


  size_type size() const {
    return std::accumulate(std::begin(_dims), std::end(_dims), 1,
      [](size_type prod, size_type dim) -> size_type {
        return prod * dim;
      });
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
    _initOffsets();
    _vec.resize(size(), std::forward<T>(val));
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, void>::type
  _assign(size_type dim1, Args&&... args) {
    _dims[N - sizeof...(Args)] = dim1;
    _assign(std::forward<Args>(args)...);
  }

  void _assign(T&& val) {
    _initOffsets();
    _vec.assign(size(), std::forward<T>(val));
  }


  void _initOffsets() {
    size_type prod = 1;
    for (size_type i = N - 1; i > 0; --i) {
      _offsets[i] = prod;
      prod *= _dims[i];
    }
    _offsets[0] = prod;
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N - 1, size_type>::type
  _indexOf(size_type dim, Args&&... args) const {
    return dim * _offsets[N - sizeof...(Args) - 1] + _indexOf(std::forward<Args>(args)...);
  }

  size_type _indexOf(size_type dim) const {
    return dim;
  }

private:
  size_type _dims[N];
  size_type _offsets[N];
  container _vec;
};

#endif // HYPERVECTOR_H
