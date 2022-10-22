#ifndef THEATRE_INPUT_H_
#define THEATRE_INPUT_H_

#include <cstring>

namespace glight::theatre {

class Controllable;

class Input {
 public:
  constexpr Input() = default;
  constexpr Input(Controllable& controllable, size_t input_index)
      : controllable_(&controllable), input_index_(input_index) {}

  constexpr Controllable* GetControllable() { return controllable_; }
  constexpr const Controllable* GetControllable() const {
    return controllable_;
  }

  constexpr void SetControllable(Controllable* controllable) {
    controllable_ = controllable;
  }

  constexpr size_t InputIndex() const { return input_index_; }

 private:
  Controllable* controllable_ = nullptr;
  size_t input_index_ = 0;
};

}  // namespace glight::theatre

#endif
