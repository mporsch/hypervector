#ifndef HYPERVECTOR_VIEW_H
#define HYPERVECTOR_VIEW_H

#include "hypervector_detail.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

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
    return offsets_[0] * sizes_[0];
  }


  template<size_type Dim>
  size_type sizeOf() const {
    static_assert(Dim < Dims, "hypervector_view::sizeOf");
    return sizes_[Dim];
  }


  template<size_type Dim>
  size_type offsetOf() const {
    static_assert(Dim < Dims, "hypervector_view::offsetOf");
    return offsets_[Dim];
  }


  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) == Dims, size_type>::type
  offsetOf(Indices&&... indices) const {
    return offsetOf_(std::forward<Indices>(indices)...);
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
    if(index0 >= sizes_[dim])
      throw std::out_of_range("hypervector_view::at");
    return index0 * offsets_[dim] + indexOf_(std::forward<Indices>(indices)...);
  }

  size_type indexOf_(size_type index) const {
    return index;
  }


  template<typename ...Indices>
  typename std::enable_if<sizeof...(Indices) <= Dims - 1, size_type>::type
  offsetOf_(
      size_type index0,
      Indices&&... indices) const {
    constexpr auto dim = Dims - sizeof...(Indices) - 1;
    return index0 * offsets_[dim] + offsetOf_(std::forward<Indices>(indices)...);
  }

  size_type offsetOf_(size_type index) const {
    return index;
  }

protected:
  const size_type* sizes_;
  const size_type* offsets_;
  iterator first_;
};

#endif // HYPERVECTOR_VIEW_H
