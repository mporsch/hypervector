#ifndef HYPERVECTOR_H
#define HYPERVECTOR_H

#include <cstddef>
#include <iostream>
#include <numeric>
#include <type_traits>
#include <vector>

template<typename T>
struct hypervector_detail
{
  using container = typename std::vector<T>;
  using size_type = typename container::size_type;
};

/// view on hypervector storage providing element read and write accessors
template<typename T, size_t N, bool IsConst>
class hypervector_view
{
public:
  using size_type = typename hypervector_detail<T>::size_type;
  using storage_pointer = typename std::conditional<IsConst, const T*, T*>::type;
  using reference = T&;
  using const_reference = const T&;
  using iterator = storage_pointer;
  using const_iterator = const T*;

public:
  hypervector_view(const size_type* dims, const size_type* offsets, storage_pointer data)
    : dims_(dims)
    , offsets_(offsets)
    , data_(data) {
  }


  // reference at(size_type pos)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, reference>::type
  at(Args&&... args) {
    return data_[indexOf_(std::forward<Args>(args)...)];
  }


  // const_reference at(size_type pos) const
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, const_reference>::type
  at(Args&&... args) const {
    return data_[indexOf_(std::forward<Args>(args)...)];
  }


  // subdimension operator[](size_type pos)
  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ > 1)>::type>
  hypervector_view<T, N - 1, IsConst>
  operator[](size_type pos) {
    return hypervector_view<T, N - 1, IsConst>(
      dims_ + 1,
      offsets_ + 1,
      data_ + pos * offsets_[0]);
  }

  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ == 1)>::type>
  reference operator[](size_type pos) {
    return *(data_ + pos);
  }


  // subdimension operator[](size_type pos) const
  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ > 1)>::type>
  hypervector_view<T, N - 1, true>
  operator[](size_type pos) const {
    return hypervector_view<T, N - 1, true>(
      dims_ + 1,
      offsets_ + 1,
      data_ + pos * offsets_[0]);
  }

  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ == 1)>::type>
  const_reference operator[](size_type pos) const {
    return *(data_ + pos);
  }


  size_type size() const {
    return std::accumulate(dims_, dims_ + N, 1,
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
    return data_;
  }


  iterator end() {
    return data_ + size();
  }


  const_iterator begin() const {
    return data_;
  }


  const_iterator end() const {
    return data_ + size();
  }


  // implicit conversion from non-const to const
  operator hypervector_view<T, N, true>() const
  {
    return hypervector_view<T, N, true>(dims_, offsets_, data_);
  }

protected:
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N - 1, size_type>::type
  indexOf_(size_type dim, Args&&... args) const {
    return dim * offsets_[N - sizeof...(Args) - 1] + indexOf_(std::forward<Args>(args)...);
  }

  size_type indexOf_(size_type dim) const {
    return dim;
  }

protected:
  const size_type* dims_;
  const size_type* offsets_;
  storage_pointer data_;
};


/// hypervector container providing storage size modifiers
template<typename T, size_t N>
class hypervector : public hypervector_view<T, N, false>
{
public:
  using base = hypervector_view<T, N, false>;
  using size_type = typename hypervector_detail<T>::size_type;
  using container = typename hypervector_detail<T>::container;

public:
  // hypervector()
  /// create uninitialized container
  hypervector()
    : base(dims_, offsets_, nullptr)
    , dims_{0}
    , offsets_{0} {
  }


  // hypervector(size_type count, const T& value)
  /// create container with given dimensions; values are initialized to given value
  template<typename ...Args>
  hypervector(
    typename std::enable_if<sizeof...(Args) == N, size_type>::type dim1,
    Args&&... args)
    : base(dims_, offsets_, nullptr) {
    assign_(dim1, std::forward<Args>(args)...);
  }


  // hypervector(size_type count)
  /// create container with given dimensions; values are default initialized
  template<typename ...Args>
  hypervector(
    typename std::enable_if<sizeof...(Args) == N - 1, size_type>::type dim1,
    Args&&... args)
    : base(dims_, offsets_, nullptr) {
    assign_(dim1, std::forward<Args>(args)..., T());
  }


  ~hypervector() {
  }


  // void resize(size_type count, const T& value)
  /// resize container to given dimensions; newly created elements will be initialized to given value
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, void>::type
  resize(size_type dim1, Args&&... args) {
    resize_(dim1, std::forward<Args>(args)...);
  }


  // void resize(size_type count)
  /// resize container to given dimensions; newly created elements will be default initialized
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N - 1, void>::type
  resize(size_type dim1, Args&&... args) {
    resize_(dim1, std::forward<Args>(args)..., T());
  }


  // void assign(size_type count, const T& value)
  /// assign given dimensions to container and set all elements to given value
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, void>::type
  assign(size_type dim1, Args&&... args) {
    assign_(dim1, std::forward<Args>(args)...);
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
    vec_.resize(this->size(), std::forward<T>(val));
    this->data_ = vec_.data();
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, void>::type
  assign_(size_type dim1, Args&&... args) {
    dims_[N - sizeof...(Args)] = dim1;
    assign_(std::forward<Args>(args)...);
  }

  void assign_(T&& val) {
    initOffsets_();
    vec_.assign(this->size(), std::forward<T>(val));
    this->data_ = vec_.data();
  }


  void initOffsets_() {
    size_type prod = 1;
    for (size_type i = N - 1; i > 0; --i) {
      offsets_[i] = prod;
      prod *= dims_[i];
    }
    offsets_[0] = prod;
  }

private:
  size_type dims_[N];
  size_type offsets_[N];
  container vec_;
};


template<typename T, size_t N, bool IsConst>
std::ostream &operator<<(std::ostream &os, const hypervector_view<T, N, IsConst>& hvec) {
  auto size = hvec.size(0);
  for(decltype(size) i = 0; i < size; ++i)
    os << "(" << hvec[i] << ")" << (i != size - 1 ? ", " : "");
  return os;
}

template<typename T, bool IsConst>
std::ostream &operator<<(std::ostream &os, const hypervector_view<T, 1, IsConst>& hvec) {
  auto size = hvec.size(0);
  for(decltype(size) i = 0; i < size; ++i)
    os << hvec[i] << (i != size - 1 ? ", " : "");
  return os;
}

#endif // HYPERVECTOR_H
