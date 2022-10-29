#ifndef UNIQUE_WITHOUT_ORDERING_H_
#define UNIQUE_WITHOUT_ORDERING_H_

#include <set>
#include <vector>

template <typename T>
std::vector<T> UniqueWithoutOrdering(const std::vector<T>& input) {
  std::vector<T> result;
  std::set<T> ordered;
  for (const T& value : input) {
    if (ordered.insert(value).second) {
      result.emplace_back(value);
    }
  }
  return result;
};

#endif
