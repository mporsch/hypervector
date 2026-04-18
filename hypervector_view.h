#ifndef HYPERVECTOR_VIEW_H
#define HYPERVECTOR_VIEW_H

#include "hypervector_detail.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

/// view on hypervector storage providing element read and write accessors
template<typename T, size_t Dims, bool IsConst>
struct hypervector_view
{
  using const_pointer = const T*;
  using pointer = T*;
  using const_reference = const T&;
  using reference = T&;
  using const_iterator = const_pointer;
  using iterator = pointer;
  using size_type = hypervector_detail::size_type;

  using dimension_storage = typename std::conditional<IsConst,
    const hypervector_detail::dimension*,
    hypervector_detail::dimension*>::type;
  using value_storage = typename std::conditional<IsConst,
    const_pointer,
    pointer>::type;
  using const_slice = typename std::conditional<(Dims > 1),
    hypervector_view<T, Dims - 1, true>,
    void>::type;
  using slice = typename std::conditional<(Dims > 1),
    hypervector_view<T, Dims - 1, IsConst>,
    void>::type;

protected:
  dimension_storage dims_;
  value_storage vals_;

public:
  hypervector_view(
      dimension_storage dims,
      value_storage vals) noexcept
    : dims_(dims)
    , vals_(vals) {
  }


  // reference at(size_type pos)
  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) == Dims, reference>::type
  at(Indices&&... indices) {
    return *(vals_ + indexOf_(std::forward<Indices>(indices)...));
  }


  // const_reference at(size_type pos) const
  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) == Dims, const_reference>::type
  at(Indices&&... indices) const {
    return *(vals_ + indexOf_(std::forward<Indices>(indices)...));
  }


  // subdimension operator[](size_type pos)
  template<size_t Dims_ = Dims,
           typename = typename std::enable_if<(Dims_ > 1)>::type>
  slice operator[](size_type pos) {
    return slice(
      dims_ + 1,
      vals_ + pos * dims_[0].offset);
  }

  template<size_t Dims_ = Dims,
           typename = typename std::enable_if<(Dims_ == 1)>::type>
  reference operator[](size_type pos) {
    return *(vals_ + pos);
  }


  // subdimension operator[](size_type pos) const
  template<size_t Dims_ = Dims,
           typename = typename std::enable_if<(Dims_ > 1)>::type>
  const_slice operator[](size_type pos) const {
    return const_slice(
      dims_ + 1,
      vals_ + pos * dims_[0].offset);
  }

  template<size_t Dims_ = Dims,
           typename = typename std::enable_if<(Dims_ == 1)>::type>
  const_reference operator[](size_type pos) const {
    return *(vals_ + pos);
  }


  size_type size() const noexcept {
    return dims_[0].offset * dims_[0].size;
  }


  template<size_type Dim>
  size_type sizeOf() const noexcept {
    static_assert(Dim < Dims, "hypervector_view::sizeOf");
    return dims_[Dim].size;
  }


  template<size_type Dim>
  size_type offsetOf() const noexcept {
    static_assert(Dim < Dims, "hypervector_view::offsetOf");
    return dims_[Dim].offset;
  }


  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) == Dims, size_type>::type
  offsetOf(Indices&&... indices) const noexcept {
    return offsetOf_(std::forward<Indices>(indices)...);
  }


  iterator begin() noexcept {
    return vals_;
  }


  iterator end() noexcept {
    return vals_ + size();
  }


  const_iterator begin() const noexcept {
    return vals_;
  }


  const_iterator end() const noexcept {
    return vals_ + size();
  }


  // implicit conversion from non-const to const
  operator hypervector_view<T, Dims, true>() const noexcept
  {
    return hypervector_view<T, Dims, true>(dims_, vals_);
  }


  // comparison of different types but equal dimensions
  template<typename U, bool IsConstO>
  bool operator==(
      const hypervector_view<U, Dims, IsConstO>& other) const {
    return true
    && std::equal(
      dims_, dims_ + Dims,
      other.dims_)
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
    constexpr auto Dim = Dims - sizeof...(Indices) - 1;
    if (index0 >= dims_[Dim].size)
      throw std::out_of_range("hypervector_view::at");
    return index0 * dims_[Dim].offset + indexOf_(std::forward<Indices>(indices)...);
  }

  size_type indexOf_() const {
    return 0;
  }


  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) <= Dims - 1, size_type>::type
  offsetOf_(
      size_type index0,
      Indices&&... indices) const noexcept {
    constexpr auto Dim = Dims - sizeof...(Indices) - 1;
    return index0 * dims_[Dim].offset + offsetOf_(std::forward<Indices>(indices)...);
  }

  size_type offsetOf_(size_type index) const noexcept {
    return index;
  }
};

#endif // HYPERVECTOR_VIEW_H
