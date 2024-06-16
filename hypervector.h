#ifndef HYPERVECTOR_H
#define HYPERVECTOR_H

#include <array>
#include <algorithm>
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

  template<size_t Size>
  static std::array<size_type, Size> make_array(const size_type& val) {
    std::array<size_type, Size> arr;
    arr.fill(val);
    return arr;
  }
};

/// view on hypervector storage providing element read and write accessors
template<typename T, size_t Dims, bool IsConst>
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
  using const_slice = typename std::conditional<(Dims > 1),
    hypervector_view<T, Dims - 1, true>,
    void>::type;
  using slice = typename std::conditional<(Dims > 1),
    hypervector_view<T, Dims - 1, IsConst>,
    void>::type;

public:
  hypervector_view() = default;


  hypervector_view(
      const size_type* sizes,
      const size_type* offsets,
      iterator first)
    : sizes_(sizes)
    , offsets_(offsets)
    , first_(first) {
  }


  // reference at(size_type pos)
  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) == Dims, reference>::type
  at(Indices&&... indices) {
    return *(first_ + indexOf_(std::forward<Indices>(indices)...));
  }


  // const_reference at(size_type pos) const
  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) == Dims, const_reference>::type
  at(Indices&&... indices) const {
    return *(first_ + indexOf_(std::forward<Indices>(indices)...));
  }


  // subdimension operator[](size_type pos)
  template<size_t Dims_ = Dims,
           typename = typename std::enable_if<(Dims_ > 1)>::type>
  slice operator[](size_type pos) {
    return slice(
      sizes_ + 1,
      offsets_ + 1,
      first_ + pos * offsets_[0]);
  }

  template<size_t Dims_ = Dims,
           typename = typename std::enable_if<(Dims_ == 1)>::type>
  reference operator[](size_type pos) {
    return *(first_ + pos);
  }


  // subdimension operator[](size_type pos) const
  template<size_t Dims_ = Dims,
           typename = typename std::enable_if<(Dims_ > 1)>::type>
  const_slice operator[](size_type pos) const {
    return const_slice(
      sizes_ + 1,
      offsets_ + 1,
      first_ + pos * offsets_[0]);
  }

  template<size_t Dims_ = Dims,
           typename = typename std::enable_if<(Dims_ == 1)>::type>
  const_reference operator[](size_type pos) const {
    return *(first_ + pos);
  }


  size_type size() const {
    return std::accumulate(sizes_, sizes_ + Dims, 1,
      [](size_type prod, size_type size) -> size_type {
        return prod * size;
      });
  }


  template<size_type Dim>
  size_type sizeOf() const {
    static_assert(Dim <= Dims, "hypervector_view::sizeOf");
    return sizes_[Dim];
  }


  template<size_type Dim>
  size_type offsetOf() const {
    static_assert(Dim <= Dims, "hypervector_view::offsetOf");
    return offsets_[Dim];
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
  operator hypervector_view<T, Dims, true>() const
  {
    return hypervector_view<T, Dims, true>(sizes_, offsets_, first_);
  }


  // comparison of different types but equal dimensions
  template<typename U, bool IsConstO>
  bool operator==(
      const hypervector_view<U, Dims, IsConstO>& other) const {
    return true
    && std::equal(
      sizes_, sizes_ + Dims,
      other.sizes_)
    && std::equal(
      begin(), end(),
      other.begin());
  }

protected:
  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) <= Dims - 1, size_type>::type
  indexOf_(
      size_type index0,
      Indices&&... indices) const {
    constexpr auto dim = Dims - sizeof...(Indices) - 1;
    if(index0 >= sizes_[dim])
      throw std::out_of_range("hypervector_view::at");
    return index0 * offsets_[dim] + indexOf_(std::forward<Indices>(indices)...);
  }

  size_type indexOf_(size_type index) const {
    return index;
  }

protected:
  const size_type* sizes_;
  const size_type* offsets_;
  iterator first_;
};


/// hypervector container providing storage size modifiers
template<typename T, size_t Dims>
class hypervector : public hypervector_view<T, Dims, false>
{
public:
  using view = hypervector_view<T, Dims, false>;
  using size_type = typename hypervector_detail<T>::size_type;
  using container = typename hypervector_detail<T>::container;

public:
  /// create uninitialized container
  hypervector()
    : sizes_{0}
    , offsets_{0} {
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
  }


  // hypervector(size_type count, const T& value)
  /// create container with given dimensions;
  /// values are initialized to given value
  template<typename ...Sizes>
  hypervector(
      typename std::enable_if<sizeof...(Sizes) == Dims, size_type>::type size0,
      Sizes&&... sizes) {
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
    (void)assign_(1, size0, std::forward<Sizes>(sizes)...);
  }


  // hypervector(size_type count)
  /// create container with given dimensions;
  /// values are default initialized
  template<typename ...Sizes>
  hypervector(
      typename std::enable_if<sizeof...(Sizes) == Dims - 1, size_type>::type size0,
      Sizes&&... sizes) {
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
    (void)assign_(1, size0, std::forward<Sizes>(sizes)..., T());
  }


  ~hypervector() {
  }


  hypervector(const hypervector& other)
    : sizes_(other.sizes_)
    , offsets_(other.offsets_)
    , vec_(other.vec_) {
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
  }


  hypervector(hypervector&& other)
    : sizes_(std::move(other.sizes_))
    , offsets_(std::move(other.offsets_))
    , vec_(std::move(other.vec_)) {
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
  }


  /// creates container with given values
  /// @note  Mind that this only sets the container values,
  ///        but not the sizes of the dimensions;
  ///        Use resize() afterwards to set these.
  hypervector(std::initializer_list<T> init)
    : sizes_(hypervector_detail<T>::template make_array<Dims>(1))
    , offsets_(hypervector_detail<T>::template make_array<Dims>(init.size()))
    , vec_(std::move(init)) {
    std::swap(sizes_[Dims - 1], offsets_[Dims - 1]);
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
  }


  hypervector& operator=(const hypervector& other) {
    sizes_ = other.sizes_;
    offsets_ = other.offsets_;
    vec_ = other.vec_;
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
    return *this;
  }


  hypervector& operator=(hypervector&& other) {
    sizes_ = std::move(other.sizes_);
    offsets_ = std::move(other.offsets_);
    vec_ = std::move(other.vec_);
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
    return *this;
  }


  // void resize(size_type count, const T& value)
  /// resize container to given dimensions;
  /// newly created elements will be initialized to given value
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims, void>::type
  resize(
      size_type size0,
      Sizes&&... sizes) {
    (void)resize_(1, size0, std::forward<Sizes>(sizes)...);
  }


  // void resize(size_type count)
  /// resize container to given dimensions;
  /// newly created elements will be default initialized
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims - 1, void>::type
  resize(
      size_type size0,
      Sizes&&... sizes) {
    (void)resize_(1, size0, std::forward<Sizes>(sizes)..., T());
  }


  // void assign(size_type count, const T& value)
  /// assign given dimensions to container and
  /// set all elements to given value
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims, void>::type
  assign(
      size_type size0,
      Sizes&&... sizes) {
    (void)assign_(1, size0, std::forward<Sizes>(sizes)...);
  }


  // void reserve(size_type count)
  /// reserve container to given maximum dimension sizes to pre-allocate storage
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims - 1, void>::type
  reserve(
      size_type size0,
      Sizes&&... sizes) {
    reserve_(1, size0, std::forward<Sizes>(sizes)...);
  }


  // void reserve(size_type count)
  /// reserve container to given maximum overall size to pre-allocate storage
  void reserve(size_type size) {
    reserve_(size);
  }

private:
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) <= Dims, size_type>::type
  resize_(
      size_type size,
      size_type size0,
      Sizes&&... sizes) {
    constexpr auto dim = Dims - sizeof...(Sizes);
    sizes_[dim] = size0;
    offsets_[dim] = resize_(size * size0, std::forward<Sizes>(sizes)...);
    return offsets_[dim] * size0;
  }

  size_type resize_(
      size_type size,
      const T& val) {
    vec_.resize(size, val);
    view::first_ = vec_.begin(); // reset iterator after possible reallocation
    return 1;
  }


  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) <= Dims, size_type>::type
  assign_(
      size_type size,
      size_type size0,
      Sizes&&... sizes) {
    constexpr auto dim = Dims - sizeof...(Sizes);
    sizes_[dim] = size0;
    offsets_[dim] = assign_(size * size0, std::forward<Sizes>(sizes)...);
    return offsets_[dim] * size0;
  }

  size_type assign_(
      size_type size,
      const T& val) {
    vec_.assign(size, val);
    view::first_ = vec_.begin(); // reset iterator after possible reallocation
    return 1;
  }


  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) < Dims, void>::type
  reserve_(
      size_type size,
      size_type size0,
      Sizes&&... sizes) {
    reserve_(size * size0, std::forward<Sizes>(sizes)...);
  }

  void reserve_(size_type size) {
    vec_.reserve(size);
    view::first_ = vec_.begin(); // reset iterator after possible reallocation
  }

private:
  std::array<size_type, Dims> sizes_;
  std::array<size_type, Dims> offsets_;
  container vec_;
};


template<typename T, size_t Dims, bool IsConst>
std::ostream& operator<<(
    std::ostream& os,
    const hypervector_view<T, Dims, IsConst>& hvec) {
  auto size = hvec.template sizeOf<0>();
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
  auto size = hvec.template sizeOf<0>();
  const char* separator = "";
  for(decltype(size) i = 0; i < size; ++i) {
    os << separator << hvec[i];
    separator = ", ";
  }
  return os;
}

#endif // HYPERVECTOR_H
