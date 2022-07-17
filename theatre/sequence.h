#ifndef THEATRE_SEQUENCE_H_
#define THEATRE_SEQUENCE_H_

#include <utility>
#include <vector>

namespace glight::theatre {

/**
        @author Andre Offringa
*/
class Sequence {
 public:
  Sequence() = default;

  size_t Size() const { return _list.size(); }

  void Add(class Controllable &controllable, size_t inputIndex) {
    _list.emplace_back(&controllable, inputIndex);
  }

  void Remove(size_t index) { _list.erase(_list.begin() + index); }

  const std::vector<std::pair<class Controllable *, size_t>> &List() const {
    return _list;
  }

  bool IsUsing(class Controllable &object) const {
    for (const std::pair<class Controllable *, size_t> &input : _list)
      if (input.first == &object) return true;
    return false;
  }

 private:
  std::vector<std::pair<class Controllable *, size_t>> _list;
};

}  // namespace glight::theatre

#endif
