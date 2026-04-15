#ifndef HYPERVECTOR_VIEW_H
#define HYPERVECTOR_VIEW_H

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

/// view on hypervector storage providing element read and write accessors
template<typename T, size_t Dims, bool IsConst>
struct hypervector_view
{
  using value_type = T;
  using const_pointer = const T*;
  using pointer = T*;
  using const_reference = const T&;
  using reference = T&;
  using const_iterator = const_pointer;
  using iterator = pointer;
  using size_type = size_t;

  using size_storage = typename std::conditional<IsConst, const size_type*, size_type*>::type;
  using value_storage = typename std::conditional<IsConst, const_pointer, pointer>::type;
  using const_slice = typename std::conditional<(Dims > 1),
    hypervector_view<T, Dims - 1, true>,
    void>::type;
  using slice = typename std::conditional<(Dims > 1),
    hypervector_view<T, Dims - 1, IsConst>,
    void>::type;

protected:
  size_storage sizes_;
  size_storage offsets_;
  value_storage first_;

public:
  hypervector_view(
      size_storage sizes,
      size_storage offsets,
      value_storage first) noexcept
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


  size_type size() const noexcept {
    return offsets_[0] * sizes_[0];
  }


  template<size_type Dim>
  size_type sizeOf() const noexcept {
    static_assert(Dim < Dims, "hypervector_view::sizeOf");
    return sizes_[Dim];
  }


  template<size_type Dim>
  size_type offsetOf() const noexcept {
    static_assert(Dim < Dims, "hypervector_view::offsetOf");
    return offsets_[Dim];
  }


  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) == Dims, size_type>::type
  offsetOf(Indices&&... indices) const noexcept {
    return offsetOf_(std::forward<Indices>(indices)...);
  }


  iterator begin() noexcept {
    return first_;
  }


  iterator end() noexcept {
    return first_ + size();
  }


  const_iterator begin() const noexcept {
    return first_;
  }


  const_iterator end() const noexcept {
    return first_ + size();
  }


  // implicit conversion from non-const to const
  operator hypervector_view<T, Dims, true>() const noexcept
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


  template<typename U, bool IsConstO>
  bool operator!=(
      const hypervector_view<U, Dims, IsConstO>& other) const {
    return !(*this == other);
  }

protected:
  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) <= Dims - 1, size_type>::type
  indexOf_(
      size_type index0,
      Indices&&... indices) const {
    constexpr auto dim = Dims - sizeof...(Indices) - 1;
    if (index0 >= sizes_[dim])
      throw std::out_of_range("hypervector_view::at");
    return index0 * offsets_[dim] + indexOf_(std::forward<Indices>(indices)...);
  }

  size_type indexOf_() const {
    return 0;
  }


  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) <= Dims - 1, size_type>::type
  offsetOf_(
      size_type index0,
      Indices&&... indices) const noexcept {
    constexpr auto dim = Dims - sizeof...(Indices) - 1;
    return index0 * offsets_[dim] + offsetOf_(std::forward<Indices>(indices)...);
  }

  size_type offsetOf_(size_type index) const noexcept {
    return index;
  }
};

#endif // HYPERVECTOR_VIEW_H
