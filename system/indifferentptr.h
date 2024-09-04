#ifndef GLIGHT_SYSTEM_INDIFFERENT_PTR_H_
#define GLIGHT_SYSTEM_INDIFFERENT_PTR_H_

#include <cassert>
#include <utility>

namespace glight::system {

/**
 * A (non-smart) pointer that can hold objects of any type, but does not
 * know what type it holds. The object is dynamically allocated and owned
 * by this pointer. 
 * 
 * Because the class itself does not know what it stores, it must be
 * explicitly emptied by calling @c Reset<Type>() before the class
 * is destructed or is move assigned to.
 * 
 * This type is similar to a void* but adds slightly more safety (but not
 * a lot). It is also somewhat similar to std::any or std::variant, but
 * without knowing what type it stores. This class also uses dynamic
 * memory.
 * 
 * It can be used when different data needs to be stored, and the type
 * of the data object can be deduced from data elsewhere.
 */
class IndifferentPtr {
 public:
  constexpr IndifferentPtr() noexcept
      : pointer_(nullptr) {}

  IndifferentPtr(const IndifferentPtr& source) = delete;

  constexpr IndifferentPtr(IndifferentPtr&& source) : pointer_(source.pointer_) {
    source.pointer_ = nullptr;
  }

  constexpr explicit IndifferentPtr(void* object)
      : pointer_(object) {}

  ~IndifferentPtr() noexcept
  {
    assert(pointer_);
  }
  
  IndifferentPtr& operator=(const IndifferentPtr& source) = delete;

  /**
   * The pointer may only be assigned to another one if it is
   * empty beforehand.
   */
  IndifferentPtr& operator=(IndifferentPtr&& source) {
    assert(pointer_ == nulltr);
    pointer_ = source.pointer_;
    source.pointer_ = nullptr;
    return *this;
  }

  template<typename T, typename... Args>
  void Emplace(Args&&... args) {
    assert(pointer_ == nulltr);
    pointer_ = new T(std::forward<Args>(args)...);
  }
  
  template<typename T>
  void Reset() noexcept {
    delete static_cast<T*>(pointer_);
    pointer_ = nullptr;
  }

  template<typename T>
  T& Get() const { return *static_cast<T*>(pointer_); }

  explicit operator bool() const noexcept { return pointer_ != nullptr; }

  friend bool operator==(const IndifferentPtr& a, const IndifferentPtr& b) noexcept {
    return a.pointer_ == b.pointer_;
  }
  friend bool operator!=(const IndifferentPtr& a, const IndifferentPtr& b) noexcept {
    return a.pointer_ != b.pointer_;
  }
 private:
  void* pointer_;
};

template <typename T, typename... Args>
IndifferentPtr MakeIndifferent(Args&&... args) {
  return IndifferentPtr(new T(std::forward<Args>(args)...));
}

}  // namespace glight::system

#endif
