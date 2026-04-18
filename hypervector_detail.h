#ifndef HYPERVECTOR_DETAIL_H
#define HYPERVECTOR_DETAIL_H

#include <cstddef>

namespace hypervector_detail {

using size_type = size_t;

struct dimension {
  size_type size;
  size_type offset;

  bool operator==(const dimension&) const noexcept = default;
};

} // namespace hypervector_detail

#endif // HYPERVECTOR_DETAIL_H
