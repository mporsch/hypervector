#ifndef HYPERVECTOR_CONTAINER_H
#define HYPERVECTOR_CONTAINER_H

#include "hypervector_detail.h"
#include "hypervector_view.h"

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <type_traits>

/// hypervector container providing size/shape modifiers and ownership of storage
template<typename T, size_t Dims>
struct hypervector : public hypervector_view<T, Dims, false>
{
  using view = hypervector_view<T, Dims, false>;

  using value_type = T;
  using size_type = typename view::size_type;

  /// uses the views members for access to shape and values
  size_type capacity_; ///< pre-allocated memory managed via reserve()

  /// create empty container
  hypervector()
    : view(new hypervector_detail::dimension[Dims](/* zero-initialized */), nullptr)
    , capacity_(0) {
  }


  // hypervector(size_type count..., const T& value)
  /// create container with given dimensions;
  /// values are initialized to given value
  template<typename ...Sizes>
  hypervector(
      typename std::enable_if<sizeof...(Sizes) == Dims, size_type>::type size0,
      Sizes&&... sizes)
    : hypervector() {
    (void)assign_(0, 1, size0, std::forward<Sizes>(sizes)...);
  }


  // hypervector(size_type count...)
  /// create container with given dimensions;
  /// values are default-initialized
  template<typename ...Sizes>
  hypervector(
      typename std::enable_if<sizeof...(Sizes) == Dims - 1, size_type>::type size0,
      Sizes&&... sizes)
    : hypervector() {
    (void)assign_(0, 1, size0, std::forward<Sizes>(sizes)..., value_type());
  }


  // hypervector(std::initializer_list<std::initializer_list<...>>)
  /// creates container with given values and dimensions
  template<typename U>
  hypervector(std::initializer_list<U> init)
    : hypervector() {
    reserve_(0, list_check_<0>(init));
    (void)list_init_<0>(0, std::move(init));
  }


  hypervector(const hypervector& other)
    : hypervector(static_cast<const view&>(other)) {
  }


  hypervector(const view& other)
    : hypervector() {
    reserve_(0, other.size());
    std::uninitialized_copy_n(other.begin(), other.size(), view::begin());
    std::copy_n(other.dims_, Dims, view::dims_);
  }


  hypervector(hypervector&& other)
    : hypervector() {
    swap(other, *this);
  }


  ~hypervector() {
    std::destroy_n(view::begin(), view::size());
    std::allocator<T>().deallocate(view::begin(), capacity());
    delete[] view::dims_;
  }


  hypervector& operator=(const hypervector& other) {
    return operator=(static_cast<const view&>(other));
  }


  hypervector& operator=(const view& other) {
    clear();
    reserve_(0, other.size());
    std::uninitialized_copy_n(other.begin(), other.size(), view::begin());
    std::copy_n(other.dims_, Dims, view::dims_);
    return *this;
  }


  hypervector& operator=(hypervector&& other) {
    clear();
    std::allocator<T>().deallocate(view::vals_, capacity_);
    view::vals_ = nullptr;
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


  /// destroy contents and set dimension sizes to zero
  void clear() {
    clear_(view::size());
  }


  // void reserve(size_type count...)
  /// reserve container to given dimension sizes to pre-allocate storage
  template<typename ...Sizes>
  typename std::enable_if<
    sizeof...(Sizes) == Dims - 1 || sizeof...(Sizes) == 0,
    void>::type
  reserve(
      size_type size0,
      Sizes&&... sizes) {
    reserve_(view::size(), size0, std::forward<Sizes>(sizes)...);
  }


  /// get current pre-allocated storage capacity
  size_type capacity() const noexcept {
    return capacity_;
  }

private:
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) <= Dims, size_type>::type
  resize_(
      size_type old_size,
      size_type acc_size,
      size_type size0,
      Sizes&&... sizes) {
    constexpr auto Dim = Dims - sizeof...(Sizes);
    view::dims_[Dim].offset = resize_(old_size, acc_size * size0, std::forward<Sizes>(sizes)...);
    view::dims_[Dim].size = size0;
    return view::dims_[Dim].offset * size0;
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
    constexpr auto Dim = Dims - sizeof...(Sizes);
    view::dims_[Dim].offset = assign_(old_size, acc_size * size0, std::forward<Sizes>(sizes)...);
    view::dims_[Dim].size = size0;
    return view::dims_[Dim].offset * size0;
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
    std::fill_n(view::dims_, Dims, hypervector_detail::dimension());
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

    auto new_vals = std::allocator<T>().allocate(new_capacity);
    std::uninitialized_move_n(view::vals_, old_size, new_vals);
    std::destroy_n(view::vals_, old_size);
    std::allocator<T>().deallocate(view::vals_, capacity_);
    view::vals_ = new_vals;
    capacity_ = new_capacity;
  }


  template<size_t Dim, typename U>
  static size_type list_check_(
      const std::initializer_list<std::initializer_list<U>>& curr) {
    static_assert(Dim < Dims, "hypervector(std::initializer_list)");

    size_type acc_size = 0;
    for (auto&& next : curr) {
      auto next_size = list_check_<Dim + 1>(next);
      if (acc_size && (acc_size != next_size))
        throw std::invalid_argument("hypervector(std::initializer_list): unequal list sizes");
      acc_size = next_size;
    }

    return acc_size * curr.size();
  }

  template<size_t Dim>
  static size_type list_check_(const std::initializer_list<T>& init) {
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
    view::dims_[Dim].offset = offset;
    view::dims_[Dim].size = curr.size();
    return view::dims_[Dim].offset * view::dims_[Dim].size;
  }

  template<size_t Dim>
  size_type list_init_(
      size_type offset,
      std::initializer_list<T> init) {
    static_assert(Dim + 1 == Dims, "hypervector(std::initializer_list)");

    auto size = init.size();
    std::uninitialized_move_n(init.begin(), size, view::begin() + offset);
    view::dims_[Dim].offset = 1;
    view::dims_[Dim].size = size;
    return size;
  }


  template<typename U, size_t Dim>
  friend void swap(hypervector<U, Dim>&, hypervector<U, Dim>&) noexcept;
};

template<typename T, size_t Dims>
void swap(hypervector<T, Dims>& lhs, hypervector<T, Dims>& rhs) noexcept
{
  using std::swap;
  swap(lhs.dims_, rhs.dims_);
  swap(lhs.vals_, rhs.vals_);
  swap(lhs.capacity_, rhs.capacity_);
}

#endif // HYPERVECTOR_CONTAINER_H
