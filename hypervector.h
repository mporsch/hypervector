#ifndef HYPERVECTOR_H
#define HYPERVECTOR_H

#include <array>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <stdexcept>
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
  using container = typename hypervector_detail<T>::container;
  using const_reference = typename container::const_reference;
  using reference = typename container::reference;
  using const_iterator = typename container::const_iterator;
  using iterator = typename std::conditional<IsConst,
    const_iterator,
    typename container::iterator>::type;

  using const_slice = typename std::conditional<(N > 1),
    hypervector_view<T, N - 1, true>,
    void>::type;
  using slice = typename std::conditional<(N > 1),
    hypervector_view<T, N - 1, IsConst>,
    void>::type;

public:
  hypervector_view() = default;


  hypervector_view(
      const size_type* dims,
      const size_type* offsets,
      iterator first)
    : dims_(dims)
    , offsets_(offsets)
    , first_(first) {
  }


  // reference at(size_type pos)
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, reference>::type
  at(Args&&... args) {
    return *(first_ + indexOf_(std::forward<Args>(args)...));
  }


  // const_reference at(size_type pos) const
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, const_reference>::type
  at(Args&&... args) const {
    return *(first_ + indexOf_(std::forward<Args>(args)...));
  }


  // subdimension operator[](size_type pos)
  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ > 1)>::type>
  slice operator[](size_type pos) {
    return slice(
      dims_ + 1,
      offsets_ + 1,
      first_ + pos * offsets_[0]);
  }

  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ == 1)>::type>
  reference operator[](size_type pos) {
    return *(first_ + pos);
  }


  // subdimension operator[](size_type pos) const
  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ > 1)>::type>
  const_slice operator[](size_type pos) const {
    return const_slice(
      dims_ + 1,
      offsets_ + 1,
      first_ + pos * offsets_[0]);
  }

  template<size_t N_ = N,
           typename = typename std::enable_if<(N_ == 1)>::type>
  const_reference operator[](size_type pos) const {
    return *(first_ + pos);
  }


  size_type size() const {
    return std::accumulate(dims_, dims_ + N, 1,
      [](size_type prod, size_type dim) -> size_type {
        return prod * dim;
      });
  }


  size_type size(size_type dim) const {
    if(dim >= N)
      throw std::out_of_range("hypervector_view::size");
    return dims_[dim];
  }


  iterator begin() {
    return first_;
  }


  iterator end() {
    return first_ + size();
  }


  const_iterator begin() const {
    return first_;
  }


  const_iterator end() const {
    return first_ + size();
  }


  // implicit conversion from non-const to const
  operator hypervector_view<T, N, true>() const
  {
    return hypervector_view<T, N, true>(dims_, offsets_, first_);
  }

protected:
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N - 1, size_type>::type
  indexOf_(
      size_type dim,
      Args&&... args) const {
    constexpr auto idx = N - sizeof...(Args) - 1;
    if(dim >= dims_[idx])
      throw std::out_of_range("hypervector_view::at");
    return dim * offsets_[idx] + indexOf_(std::forward<Args>(args)...);
  }

  size_type indexOf_(size_type dim) const {
    return dim;
  }

protected:
  const size_type* dims_;
  const size_type* offsets_;
  iterator first_;
};


/// hypervector container providing storage size modifiers
template<typename T, size_t N>
class hypervector : public hypervector_view<T, N, false>
{
public:
  using view = hypervector_view<T, N, false>;
  using size_type = typename hypervector_detail<T>::size_type;
  using container = typename hypervector_detail<T>::container;

public:
  // hypervector()
  /// create uninitialized container
  hypervector()
    : dims_{0}
    , offsets_{0} {
    static_cast<view&>(*this) = view(dims_.data(), offsets_.data(), vec_.begin());
  }


  // hypervector(size_type count, const T& value)
  /// create container with given dimensions;
  /// values are initialized to given value
  template<typename ...Args>
  hypervector(
      typename std::enable_if<sizeof...(Args) == N, size_type>::type dim0,
      Args&&... args) {
    static_cast<view&>(*this) = view(dims_.data(), offsets_.data(), vec_.begin());
    (void)assign_(1, dim0, std::forward<Args>(args)...);
  }


  // hypervector(size_type count)
  /// create container with given dimensions;
  /// values are default initialized
  template<typename ...Args>
  hypervector(
      typename std::enable_if<sizeof...(Args) == N - 1, size_type>::type dim0,
      Args&&... args) {
    static_cast<view&>(*this) = view(dims_.data(), offsets_.data(), vec_.begin());
    (void)assign_(1, dim0, std::forward<Args>(args)..., T());
  }


  ~hypervector() {
  }


  hypervector(const hypervector& other)
    : dims_(other.dims_)
    , offsets_(other.offsets_)
    , vec_(other.vec_) {
    static_cast<view&>(*this) = view(dims_.data(), offsets_.data(), vec_.begin());
  }


  hypervector(hypervector&& other)
    : dims_(std::move(other.dims_))
    , offsets_(std::move(other.offsets_))
    , vec_(std::move(other.vec_)) {
    static_cast<view&>(*this) = view(dims_.data(), offsets_.data(), vec_.begin());
  }


  hypervector& operator=(const hypervector& other) {
    dims_ = other.dims_;
    offsets_ = other.offsets_;
    vec_ = other.vec_;
    static_cast<view&>(*this) = view(dims_.data(), offsets_.data(), vec_.begin());
    return *this;
  }


  hypervector& operator=(hypervector&& other) {
    dims_ = std::move(other.dims_);
    offsets_ = std::move(other.offsets_);
    vec_ = std::move(other.vec_);
    static_cast<view&>(*this) = view(dims_.data(), offsets_.data(), vec_.begin());
    return *this;
  }


  // void resize(size_type count, const T& value)
  /// resize container to given dimensions;
  /// newly created elements will be initialized to given value
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, void>::type
  resize(
      size_type dim0,
      Args&&... args) {
    (void)resize_(1, dim0, std::forward<Args>(args)...);
  }


  // void resize(size_type count)
  /// resize container to given dimensions;
  /// newly created elements will be default initialized
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N - 1, void>::type
  resize(
      size_type dim0,
      Args&&... args) {
    (void)resize_(1, dim0, std::forward<Args>(args)..., T());
  }


  // void assign(size_type count, const T& value)
  /// assign given dimensions to container and
  /// set all elements to given value
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N, void>::type
  assign(
      size_type dim0,
      Args&&... args) {
    (void)assign_(1, dim0, std::forward<Args>(args)...);
  }


  // void reserve(size_type count)
  /// reserve container to given maximum dimension sizes to pre-allocate storage
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) == N - 1, void>::type
  reserve(
      size_type dim0,
      Args&&... args) {
    reserve_(1, dim0, std::forward<Args>(args)...);
  }


  // void reserve(size_type count)
  /// reserve container to given maximum overall size to pre-allocate storage
  void reserve(size_type size) {
    reserve_(size);
  }

private:
  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, size_type>::type
  resize_(
      size_type size,
      size_type dim,
      Args&&... args) {
    constexpr auto idx = N - sizeof...(Args);
    dims_[idx] = dim;
    offsets_[idx] = resize_(size * dim, std::forward<Args>(args)...);
    return offsets_[idx] * dim;
  }

  size_type resize_(
      size_type size,
      const T& val) {
    vec_.resize(size, val);
    view::first_ = vec_.begin(); // reset iterator after possible reallocation
    return 1;
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) <= N, size_type>::type
  assign_(
      size_type size,
      size_type dim,
      Args&&... args) {
    constexpr auto idx = N - sizeof...(Args);
    dims_[idx] = dim;
    offsets_[idx] = assign_(size * dim, std::forward<Args>(args)...);
    return offsets_[idx] * dim;
  }

  size_type assign_(
      size_type size,
      const T& val) {
    vec_.assign(size, val);
    view::first_ = vec_.begin(); // reset iterator after possible reallocation
    return 1;
  }


  template<typename ...Args>
  typename std::enable_if<sizeof...(Args) < N, void>::type
  reserve_(
      size_type size,
      size_type dim,
      Args&&... args) {
    reserve_(size * dim, std::forward<Args>(args)...);
  }

  void reserve_(size_type size) {
    vec_.reserve(size);
    view::first_ = vec_.begin(); // reset iterator after possible reallocation
  }

private:
  std::array<size_type, N> dims_;
  std::array<size_type, N> offsets_;
  container vec_;
};


template<typename T, size_t N, bool IsConst>
std::ostream& operator<<(
    std::ostream& os,
    const hypervector_view<T, N, IsConst>& hvec) {
  auto size = hvec.size(0);
  const char* separator = "";
  for(decltype(size) i = 0; i < size; ++i) {
    os << separator << "(" << hvec[i] << ")";
    separator = ", ";
  }
  return os;
}

template<typename T, bool IsConst>
std::ostream& operator<<(
    std::ostream& os,
    const hypervector_view<T, 1, IsConst>& hvec) {
  auto size = hvec.size(0);
  const char* separator = "";
  for(decltype(size) i = 0; i < size; ++i) {
    os << separator << hvec[i];
    separator = ", ";
  }
  return os;
}

#endif // HYPERVECTOR_H
