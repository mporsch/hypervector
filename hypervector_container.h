#ifndef HYPERVECTOR_CONTAINER_H
#define HYPERVECTOR_CONTAINER_H

#include "hypervector_view.h"

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <type_traits>

/// hypervector container providing storage size modifiers
template<typename T, size_t Dims>
struct hypervector : public hypervector_view<T, Dims, false>
{
  using view = hypervector_view<T, Dims, false>;

  using value_type = typename view::value_type;
  using size_type = typename view::size_type;

  size_type capacity_;

  /// create empty container
  hypervector()
    : hypervector(new size_t[Dims * 2]()) { // zero-initialized
  }


  // hypervector(size_type count..., const T& value)
  /// create container with given dimensions;
  /// values are initialized to given value
  template<typename ...Sizes>
  hypervector(
      typename std::enable_if<sizeof...(Sizes) == Dims, size_type>::type size0,
      Sizes&&... sizes)
    : hypervector(new size_t[Dims * 2]()) { // zero-initialized
    (void)assign_(0, 1, size0, std::forward<Sizes>(sizes)...);
  }


  // hypervector(size_type count...)
  /// create container with given dimensions;
  /// values are default-initialized
  template<typename ...Sizes>
  hypervector(
      typename std::enable_if<sizeof...(Sizes) == Dims - 1, size_type>::type size0,
      Sizes&&... sizes)
    : hypervector(new size_t[Dims * 2]()) { // zero-initialized
    (void)assign_(0, 1, size0, std::forward<Sizes>(sizes)..., value_type());
  }


  ~hypervector() {
    std::destroy_n(view::begin(), view::size());
    std::allocator<T>().deallocate(view::begin(), capacity_);
    delete[] view::sizes_; // offsets_ belongs to the same allocation
  }


  hypervector(const hypervector& other)
    : hypervector(new size_t[Dims * 2]()) { // zero-initialized
    reserve_(0, other.size());
    std::uninitialized_copy_n(other.begin(), other.size(), view::begin());
    std::copy_n(other.sizes_, Dims * 2, view::sizes_); // offsets_ is included
  }


  hypervector(hypervector&& other)
    : hypervector(new size_t[Dims * 2]()) { // zero-initialized
    swap(other, *this);
  }


  // hypervector(std::initializer_list<std::initializer_list<...>>)
  /// creates container with given values and dimensions
  template<typename U>
  hypervector(std::initializer_list<U> init)
    : hypervector(new size_t[Dims * 2]()) { // zero-initialized
    reserve_(0, list_check_<0>(init));
    (void)list_init_<0>(0, std::move(init));
  }


  hypervector& operator=(const hypervector& other) {
    clear();
    reserve_(0, other.size());
    std::uninitialized_copy_n(other.begin(), other.size(), view::begin());
    std::copy_n(other.sizes_, Dims * 2, view::sizes_); // offsets_ is included
    return *this;
  }


  hypervector& operator=(hypervector&& other) {
    clear();
    std::allocator<T>().deallocate(view::first_, capacity_);
    view::first_ = nullptr;
    capacity_ = 0;

    swap(other, *this);
    return *this;
  }


  // void resize(size_type count..., const T& value)
  /// resize container to given dimensions;
  /// newly created elements will be initialized to given value
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims, void>::type
  resize(
      size_type size0,
      Sizes&&... sizes) {
    (void)resize_(view::size(), 1, size0, std::forward<Sizes>(sizes)...);
  }


  // void resize(size_type count...)
  /// resize container to given dimensions;
  /// newly created elements will be default-initialized
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims - 1, void>::type
  resize(
      size_type size0,
      Sizes&&... sizes) {
    (void)resize_(view::size(), 1, size0, std::forward<Sizes>(sizes)..., T());
  }


  // void assign(size_type count..., const T& value)
  /// assign given dimensions to container and
  /// set all elements to given value
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims, void>::type
  assign(
      size_type size0,
      Sizes&&... sizes) {
    (void)assign_(view::size(), 1, size0, std::forward<Sizes>(sizes)...);
  }


  void clear() {
    clear_(view::size());
  }


  // void reserve(size_type count...)
  /// reserve container to given maximum dimension sizes to pre-allocate storage
  template<typename ...Sizes>
  typename std::enable_if<
    sizeof...(Sizes) == Dims - 1 || sizeof...(Sizes) == 0,
    void>::type
  reserve(
      size_type size0,
      Sizes&&... sizes) {
    reserve_(view::size(), size0, std::forward<Sizes>(sizes)...);
  }

  size_type capacity() const noexcept {
    return capacity_;
  }

private:
  hypervector(size_type* storage) noexcept
    : view(storage, storage + Dims, nullptr)
    , capacity_(0) {
  }

  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) <= Dims, size_type>::type
  resize_(
      size_type old_size,
      size_type acc_size,
      size_type size0,
      Sizes&&... sizes) {
    constexpr auto dim = Dims - sizeof...(Sizes);
    view::offsets_[dim] = resize_(old_size, acc_size * size0, std::forward<Sizes>(sizes)...);
    view::sizes_[dim] = size0;
    return view::offsets_[dim] * size0;
  }

  size_type resize_(
      size_type old_size,
      size_type new_size,
      const T& val) {
    if (new_size > old_size) {
      reserve_(old_size, new_size);
      std::uninitialized_fill_n(view::begin() + old_size, new_size - old_size, val);
    } else if (old_size > new_size) {
      std::destroy_n(view::begin() + new_size, old_size - new_size);
    }
    return 1;
  }


  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) <= Dims, size_type>::type
  assign_(
      size_type old_size,
      size_type acc_size,
      size_type size0,
      Sizes&&... sizes) {
    constexpr auto dim = Dims - sizeof...(Sizes);
    view::offsets_[dim] = assign_(old_size, acc_size * size0, std::forward<Sizes>(sizes)...);
    view::sizes_[dim] = size0;
    return view::offsets_[dim] * size0;
  }

  size_type assign_(
      size_type old_size,
      size_type new_size,
      const T& val) {
    // no need to preserve anything: destroy, possibly grow, overwrite
    clear_(old_size);
    reserve_(0, new_size);
    std::uninitialized_fill_n(view::begin(), new_size, val);
    return 1;
  }


  void clear_(size_type old_size) {
    std::destroy_n(view::begin(), old_size);
    std::fill_n(view::sizes_, Dims * 2, 0); // offsets_ is included
  }


  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) < Dims, void>::type
  reserve_(
      size_type old_size,
      size_type acc_capacity,
      size_type size0,
      Sizes&&... sizes) {
    reserve_(old_size, acc_capacity * size0, std::forward<Sizes>(sizes)...);
  }

  void reserve_(
      size_type old_size,
      size_type new_capacity) {
    if (new_capacity <= capacity_)
      return;

    auto new_first = std::allocator<T>().allocate(new_capacity);
    std::uninitialized_move_n(view::first_, old_size, new_first);
    std::destroy_n(view::first_, old_size);
    std::allocator<T>().deallocate(view::first_, capacity_);
    view::first_ = new_first;
    capacity_ = new_capacity;
  }


  template<size_t Dim, typename U>
  size_type list_check_(
      const std::initializer_list<std::initializer_list<U>>& curr) {
    static_assert(Dim < Dims, "hypervector(std::initializer_list)");

    size_type acc_size = 0;
    for (auto&& next : curr) {
      auto next_size = list_check_<Dim + 1>(next);
      if (acc_size && (acc_size != next_size)) {
        throw std::invalid_argument("hypervector(std::initializer_list): unequal list sizes");
      }
      acc_size = next_size;
    }

    return acc_size * curr.size();
  }

  template<size_t Dim>
  size_type list_check_(const std::initializer_list<T>& init) {
    static_assert(Dim + 1 == Dims, "hypervector(std::initializer_list)");

    return init.size();
  }


  template<size_t Dim, typename U>
  size_type list_init_(
      size_type acc_offset,
      std::initializer_list<std::initializer_list<U>> curr) {
    static_assert(Dim < Dims, "hypervector(std::initializer_list)");

    size_type offset = 0;
    for (auto&& next : curr) {
      offset = list_init_<Dim + 1>(acc_offset, std::move(next));
      acc_offset += offset;
    }

    // XXX offsets and sizes are applied before all values have been moved
    //     not ideal but the critical top-level offset and size are written last
    view::offsets_[Dim] = offset;
    view::sizes_[Dim] = curr.size();
    return view::offsets_[Dim] * view::sizes_[Dim];
  }

  template<size_t Dim>
  size_type list_init_(
      size_type offset,
      std::initializer_list<T> init) {
    static_assert(Dim + 1 == Dims, "hypervector(std::initializer_list)");

    auto size = init.size();
    std::uninitialized_move_n(init.begin(), size, view::begin() + offset);
    view::offsets_[Dim] = 1;
    view::sizes_[Dim] = size;
    return size;
  }


  template<typename U, size_t Dim>
  friend void swap(hypervector<U, Dim>&, hypervector<U, Dim>&) noexcept;
};

template<typename T, size_t Dims>
void swap(hypervector<T, Dims>& lhs, hypervector<T, Dims>& rhs) noexcept
{
  using std::swap;
  swap(lhs.sizes_, rhs.sizes_);
  swap(lhs.offsets_, rhs.offsets_);
  swap(lhs.first_, rhs.first_);
  swap(lhs.capacity_, rhs.capacity_);
}

#endif // HYPERVECTOR_CONTAINER_H
