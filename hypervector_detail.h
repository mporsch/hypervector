#ifndef HYPERVECTOR_DETAIL_H
#define HYPERVECTOR_DETAIL_H

#include <vector>

template<typename T>
struct hypervector_detail
{
  using container = typename std::vector<T>;
  using size_type = typename container::size_type;
};

#endif // HYPERVECTOR_DETAIL_H
