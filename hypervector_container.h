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
    : hypervector(
        std::make_unique<size_type[]>(Dims), // zero-initialized
        std::make_unique<size_type[]>(Dims)
      ) {
  }


  // hypervector(size_type count..., const T& value)
  /// create container with given dimensions;
  /// values are initialized to given value
  template<typename ...Sizes>
  hypervector(
      typename std::enable_if<sizeof...(Sizes) == Dims, size_type>::type size0,
      Sizes&&... sizes)
    : hypervector(
        std::make_unique_for_overwrite<size_type[]>(Dims),
        std::make_unique_for_overwrite<size_type[]>(Dims)
      ) {
    (void)assign_(0, 1, size0, std::forward<Sizes>(sizes)...);
  }


  // hypervector(size_type count...)
  /// create container with given dimensions;
  /// values are default-initialized
  template<typename ...Sizes>
  hypervector(
      typename std::enable_if<sizeof...(Sizes) == Dims - 1, size_type>::type size0,
      Sizes&&... sizes)
    : hypervector(
        std::make_unique_for_overwrite<size_type[]>(Dims),
        std::make_unique_for_overwrite<size_type[]>(Dims)
      ) {
    (void)assign_(0, 1, size0, std::forward<Sizes>(sizes)..., value_type());
  }


  ~hypervector() noexcept {
    std::destroy_n(view::begin(), view::size());
    std::allocator<T>().deallocate(view::first_, capacity_);
    delete[] view::offsets_;
    delete[] view::sizes_;
  }


  hypervector(const hypervector& other)
    : hypervector(
        std::make_unique_for_overwrite<size_type[]>(Dims),
        std::make_unique_for_overwrite<size_type[]>(Dims)
      ) {
    std::copy_n(other.sizes_, Dims, view::sizes_);
    std::copy_n(other.offsets_, Dims, view::offsets_);
    reserve_(0, other.size());
    std::uninitialized_copy_n(other.first_, other.size(), view::first_);
  }


  hypervector(hypervector&& other) noexcept
    : view(
        other.sizes_,
        other.offsets_,
        other.first_
      )
    , capacity_(other.capacity_) {
    other.sizes_ = nullptr;
    other.offsets_ = nullptr;
    other.first_ = nullptr;
  }


  // hypervector(std::initializer_list<std::initializer_list<...>>)
  /// creates container with given values and dimensions
  template<typename U>
  hypervector(std::initializer_list<U> init)
    : hypervector(
        std::make_unique<size_type[]>(Dims), // zero-initialized
        std::make_unique<size_type[]>(Dims)
      ) {
    reserve_(0, list_init_size_<0>(init));
    list_init_values_<0>(0, std::move(init));
  }


  hypervector& operator=(const hypervector& other) {
    // no need to preserve anything: destroy, possibly grow, overwrite
    std::destroy_n(view::first_, view::size());
    reserve_(0, other.size());
    std::copy_n(other.sizes_, Dims, view::sizes_);
    std::copy_n(other.offsets_, Dims, view::offsets_);
    std::uninitialized_copy_n(other.first_, other.size(), view::first_);
    return *this;
  }


  hypervector& operator=(hypervector&& other) {
    std::destroy_at(this);
    view::sizes_ = other.sizes_;
    view::offsets_ = other.offsets_;
    view::first_ = other.first_;
    capacity_ = other.capacity_;
    other.sizes_ = nullptr;
    other.offsets_ = nullptr;
    other.first_ = nullptr;
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
  hypervector(
      std::unique_ptr<size_type[]> sizes,
      std::unique_ptr<size_type[]> offsets)
    : view(sizes.release(), offsets.release(), nullptr)
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
    view::sizes_[dim] = size0;
    view::offsets_[dim] = resize_(old_size, acc_size * size0, std::forward<Sizes>(sizes)...);
    return view::offsets_[dim] * size0;
  }

  size_type resize_(
      size_type old_size,
      size_type new_size,
      const T& val) {
    if (new_size > old_size) {
      reserve_(old_size, new_size);
      std::uninitialized_fill_n(view::first_ + old_size, new_size - old_size, val);
    } else if (old_size > new_size) {
      std::destroy_n(view::first_ + new_size, old_size - new_size);
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
    view::sizes_[dim] = size0;
    view::offsets_[dim] = assign_(old_size, acc_size * size0, std::forward<Sizes>(sizes)...);
    return view::offsets_[dim] * size0;
  }

  size_type assign_(
      size_type old_size,
      size_type new_size,
      const T& val) {
    // no need to preserve anything: destroy, possibly grow, overwrite
    std::destroy_n(view::first_, old_size);
    reserve_(0, new_size);
    std::uninitialized_fill_n(view::first_, new_size, val);
    return 1;
  }


  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) < Dims, void>::type
  reserve_(
      size_type old_size,
      size_type size,
      size_type size0,
      Sizes&&... sizes) {
    reserve_(old_size, size * size0, std::forward<Sizes>(sizes)...);
  }

  void reserve_(
      size_type old_size,
      size_type size) {
    if (size <= capacity_)
      return;

    auto ptr = std::allocator<T>().allocate(size);
    std::uninitialized_move_n(view::first_, old_size, ptr);
    std::destroy_n(view::first_, old_size);
    std::allocator<T>().deallocate(view::first_, capacity_);
    view::first_ = ptr;
    capacity_ = size;
  }


  template<size_t Dim, typename U>
  size_type list_init_size_(
      const std::initializer_list<std::initializer_list<U>>& curr) {
    static_assert(Dim < Dims, "hypervector(std::initializer_list)");

    auto curr_size = curr.size();
    view::sizes_[Dim] = curr_size;

    for (auto&& next : curr) {
      auto next_size = view::sizes_[Dim + 1];
      if (next_size && (next_size != next.size())) {
        throw std::invalid_argument("hypervector(std::initializer_list): unequal list sizes");
      }

      view::offsets_[Dim] = list_init_size_<Dim + 1>(next);
    }

    return view::offsets_[Dim] * curr_size;
  }

  template<size_t Dim>
  size_type list_init_size_(const std::initializer_list<T>& init) {
    static_assert(Dim + 1 == Dims, "hypervector(std::initializer_list)");

    auto size = init.size();
    view::sizes_[Dim] = size;
    view::offsets_[Dim] = 1;
    return size;
  }


  template<size_t Dim, typename U>
  void list_init_values_(
      size_type offset,
      std::initializer_list<std::initializer_list<U>> curr) {
    static_assert(Dim < Dims, "hypervector(std::initializer_list)");

    for (auto&& next : curr) {
      list_init_values_<Dim + 1>(offset, std::move(next));
      offset += view::offsets_[Dim];
    }
  }

  template<size_t Dim>
  void list_init_values_(
      size_type offset,
      std::initializer_list<T> init) {
    static_assert(Dim + 1 == Dims, "hypervector(std::initializer_list)");
    std::uninitialized_move_n(init.begin(), init.size(), view::first_ + offset);
  }
};

#endif // HYPERVECTOR_CONTAINER_H
