#ifndef HYPERVECTOR_CONTAINER_H
#define HYPERVECTOR_CONTAINER_H

#include "hypervector_detail.h"
#include "hypervector_view.h"

#include <array>
#include <cstddef>
#include <type_traits>

/// hypervector container providing storage size modifiers
template<typename T, size_t Dims>
class hypervector : public hypervector_view<T, Dims, false>
{
public:
  using view = hypervector_view<T, Dims, false>;
  using size_type = typename hypervector_detail<T>::size_type;
  using container = typename hypervector_detail<T>::container;

public:
  /// create empty container
  hypervector()
    : sizes_{0}
    , offsets_{0} {
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
  }


  // hypervector(size_type count..., const T& value)
  /// create container with given dimensions;
  /// values are initialized to given value
  template<typename ...Sizes>
  hypervector(
      typename std::enable_if<sizeof...(Sizes) == Dims, size_type>::type size0,
      Sizes&&... sizes) {
    static_cast<view&>(*this) = view(sizes_.data(), offsets_.data(), vec_.begin());
    (void)assign_(1, size0, std::forward<Sizes>(sizes)...);
  }


  // hypervector(size_type count...)
  /// create container with given dimensions;
  /// values are default-initialized
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


  // hypervector(std::initializer_list<std::initializer_list<...>>)
  /// creates container with given values and dimensions
  template<typename U>
  hypervector(std::initializer_list<U> init)
    : sizes_{0}
    , offsets_{0} {
    vec_.reserve(list_init_size_<0>(init));
    list_init_values_<0>(std::move(init));
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


  // void resize(size_type count..., const T& value)
  /// resize container to given dimensions;
  /// newly created elements will be initialized to given value
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims, void>::type
  resize(
      size_type size0,
      Sizes&&... sizes) {
    (void)resize_(1, size0, std::forward<Sizes>(sizes)...);
  }


  // void resize(size_type count...)
  /// resize container to given dimensions;
  /// newly created elements will be default-initialized
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims - 1, void>::type
  resize(
      size_type size0,
      Sizes&&... sizes) {
    (void)resize_(1, size0, std::forward<Sizes>(sizes)..., T());
  }


  // void assign(size_type count..., const T& value)
  /// assign given dimensions to container and
  /// set all elements to given value
  template<typename ...Sizes>
  typename std::enable_if<sizeof...(Sizes) == Dims, void>::type
  assign(
      size_type size0,
      Sizes&&... sizes) {
    (void)assign_(1, size0, std::forward<Sizes>(sizes)...);
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
    reserve_(1, size0, std::forward<Sizes>(sizes)...);
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


  template<size_t Dim, typename U>
  size_type list_init_size_(
      const std::initializer_list<std::initializer_list<U>>& curr) {
    static_assert(Dim < Dims, "hypervector(std::initializer_list)");

    auto curr_size = curr.size();
    sizes_[Dim] = curr_size;

    for(auto&& next : curr) {
      auto next_size = sizes_[Dim + 1];
      if(next_size && (next_size != next.size())) {
        throw std::invalid_argument("hypervector(std::initializer_list): unequal list sizes");
      }

      offsets_[Dim] = list_init_size_<Dim + 1>(next);
    }

    return offsets_[Dim] * curr_size;
  }

  template<size_t Dim>
  size_type list_init_size_(const std::initializer_list<T>& init) {
    static_assert(Dim + 1 == Dims, "hypervector(std::initializer_list)");

    auto size = init.size();
    sizes_[Dim] = size;
    offsets_[Dim] = 1;
    return size;
  }


  template<size_t Dim, typename U>
  void list_init_values_(
      std::initializer_list<std::initializer_list<U>> curr) {
    static_assert(Dim < Dims, "hypervector(std::initializer_list)");

    for(auto&& next : curr) {
      list_init_values_<Dim + 1>(std::move(next));
    }
  }

  template<size_t Dim>
  void list_init_values_(std::initializer_list<T> init) {
    static_assert(Dim + 1 == Dims, "hypervector(std::initializer_list)");
    vec_.insert(end(vec_), std::move(init));
  }

private:
  std::array<size_type, Dims> sizes_;
  std::array<size_type, Dims> offsets_;
  container vec_;
};

#endif // HYPERVECTOR_CONTAINER_H
