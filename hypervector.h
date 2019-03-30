#ifndef HYPERVECTOR_H
#define HYPERVECTOR_H

#include <cstddef>
#include <iostream>
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
      : dims_(dims)
      , offsets_(offsets)
      , data_(data) {
    }


    // operator[](size_type pos)
    template<size_t N_ = N,
             typename = typename std::enable_if<(N_ > 1)>::type>
    subdimension<T, N - 1>
    operator[](size_type pos) {
      return subdimension<T, N - 1>(
        dims_ + 1,
        offsets_ + 1,
        data_ + pos * offsets_[0]);
    }

    template<size_t N_ = N,
             typename = typename std::enable_if<(N_ == 1)>::type>
    reference operator[](size_type pos) {
      return *(data_ + pos);
    }


    // operator[](size_type pos) const
    template<size_t N_ = N,
             typename = typename std::enable_if<(N_ > 1)>::type>
    const subdimension<T, N - 1>
    operator[](size_type pos) const {
      return subdimension<T, N - 1>(
        dims_ + 1,
        offsets_ + 1,
        data_ + pos * offsets_[0]);
    }

    template<size_t N_ = N,
             typename = typename std::enable_if<(N_ == 1)>::type>
    const_reference operator[](size_type pos) const {
      return *(data_ + pos);
    }


    size_type size(size_type dim) const {
      if (dim < N)
        return dims_[dim];
      else
        return 0;
    }

  private:
    const size_type* dims_;
    const size_type* offsets_;
    T* data_;
  };

  template<typename T, size_t N>
  std::ostream &operator<<(std::ostream &os, const subdimension<T, N>& subd) {
    auto size = subd.size(0);
    for(decltype(size) i = 0; i < size; ++i)
      os << "(" << subd[i] << ")" << (i != size - 1 ? ", " : "");
    return os;
  }

  template<typename T>
  std::ostream &operator<<(std::ostream &os, const subdimension<T, 1>& subd) {
    auto size = subd.size(0);
    for(decltype(size) i = 0; i < size; ++i)
      os << subd[i] << (i != size - 1 ? ", " : "");
    return os;
  }
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
    : dims_{0}
    , offsets_{0} {
  }


  // hypervector(size_type count, const T& value)
  template<typename ...Args>
  hypervector(
    typename std::enable_if<sizeof...(Args) == N, size_type>::type dim1,
    Args&&... args) {
    assign_(dim1, std::forward<Args>(args)...);
  }


  // hypervector(size_type count)
  template<typename ...Args>
  hypervector(
    typename std::enable_if<sizeof...(Args) == N - 1, size_type>::type dim1,
    Args&&... args) {
    assign_(dim1, std::forward<Args>(args)..., T());
  }


  ~hypervector() {
  }


  // resize(size_type count, const T& value)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, void>::type
  resize(size_type dim1, Args&&... args) {
    resize_(dim1, std::forward<Args>(args)...);
  }


  // resize(size_type count)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N - 1, void>::type
  resize(size_type dim1, Args&&... args) {
    resize_(dim1, std::forward<Args>(args)..., T());
  }


  // assign(size_type count, const T& value)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, void>::type
  assign(size_type dim1, Args&&... args) {
    assign_(dim1, std::forward<Args>(args)...);
  }


  // at(size_type pos)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, reference>::type
  at(Args&&... args) {
    return vec_.at(indexOf_(std::forward<Args>(args)...));
  }


  // at(size_type pos) const
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, const_reference>::type
  at(Args&&... args) const {
    return vec_.at(indexOf_(std::forward<Args>(args)...));
  }


  // operator[](size_type pos)
  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ > 1)>::type>
  hypervector_detail::subdimension<T, N - 1>
  operator[](size_type pos) {
    return hypervector_detail::subdimension<T, N - 1>(
      dims_ + 1,
      offsets_ + 1,
      vec_.data() + pos * offsets_[0]);
  }

  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ == 1)>::type>
  reference operator[](size_type pos) {
    return vec_[pos];
  }


  // operator[](size_type pos) const
  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ > 1)>::type>
  const hypervector_detail::subdimension<T, N - 1>
  operator[](size_type pos) const {
    return hypervector_detail::subdimension<T, N - 1>(
      dims_ + 1,
      offsets_ + 1,
      const_cast<T *>(vec_.data()) + pos * offsets_[0]);
  }

  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ == 1)>::type>
  const_reference operator[](size_type pos) const {
    return vec_[pos];
  }


  size_type size() const {
    return std::accumulate(std::begin(dims_), std::end(dims_), 1,
      [](size_type prod, size_type dim) -> size_type {
        return prod * dim;
      });
  }


  size_type size(size_type dim) const {
    if (dim < N)
      return dims_[dim];
    else
      return 0;
  }


  iterator begin() {
    return vec_.begin();
  }


  iterator end() {
    return vec_.end();
  }


  const_iterator begin() const {
    return vec_.cbegin();
  }


  const_iterator end() const {
    return vec_.cend();
  }

private:
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, void>::type
  resize_(size_type dim1, Args&&... args) {
    dims_[N - sizeof...(Args)] = dim1;
    resize_(std::forward<Args>(args)...);
  }

  void resize_(T&& val) {
    initOffsets_();
    vec_.resize(size(), std::forward<T>(val));
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, void>::type
  assign_(size_type dim1, Args&&... args) {
    dims_[N - sizeof...(Args)] = dim1;
    assign_(std::forward<Args>(args)...);
  }

  void assign_(T&& val) {
    initOffsets_();
    vec_.assign(size(), std::forward<T>(val));
  }


  void initOffsets_() {
    size_type prod = 1;
    for (size_type i = N - 1; i > 0; --i) {
      offsets_[i] = prod;
      prod *= dims_[i];
    }
    offsets_[0] = prod;
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N - 1, size_type>::type
  indexOf_(size_type dim, Args&&... args) const {
    return dim * offsets_[N - sizeof...(Args) - 1] + indexOf_(std::forward<Args>(args)...);
  }

  size_type indexOf_(size_type dim) const {
    return dim;
  }

private:
  size_type dims_[N];
  size_type offsets_[N];
  container vec_;
};

template<typename T, size_t N>
std::ostream &operator<<(std::ostream &os, const hypervector<T, N>& hvec) {
  auto size = hvec.size(0);
  for(decltype(size) i = 0; i < size; ++i)
    os << "(" << hvec[i] << ")" << (i != size - 1 ? ", " : "");
  return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const hypervector<T, 1>& hvec) {
  auto size = hvec.size(0);
  for(decltype(size) i = 0; i < size; ++i)
    os << hvec[i] << (i != size - 1 ? ", " : "");
  return os;
}

#endif // HYPERVECTOR_H
