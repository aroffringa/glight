#ifndef THEATRE_SEQUENCE_H_
#define THEATRE_SEQUENCE_H_

#include <utility>
#include <vector>

#include "input.h"

namespace glight::theatre {

class Controllable;

class Sequence {
 public:
  Sequence() = default;

  size_t Size() const { return list_.size(); }

  void Add(Controllable &controllable, size_t inputIndex) {
    list_.emplace_back(controllable, inputIndex);
  }

  void Remove(size_t index) { list_.erase(list_.begin() + index); }

  const std::vector<Input> &List() const { return list_; }
  std::vector<Input> &List() { return list_; }

  bool IsUsing(Controllable &object) const {
    for (const Input &input : list_)
      if (input.GetControllable() == &object) return true;
    return false;
  }

 private:
  std::vector<Input> list_;
};

}  // namespace glight::theatre

#endif
