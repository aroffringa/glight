#ifndef GLIGHT_SYSTEM_DELETABLE_PTR_H_
#define GLIGHT_SYSTEM_DELETABLE_PTR_H_

#include <memory>

namespace glight::system {

/**
 * A smart shared pointer that can delete the object even
 * when pointers to the object still exist. When DeletablePtrs
 * point to the same object, and the object is deleted
 * by using the @ref DeletablePtr::Delete() method, all
 * other DeletablePtrs pointing to this object will be reset.
 */
template <typename T>
class DeletablePtr {
 public:
  using value_type = T;
  using pointer_type = T*;

  constexpr DeletablePtr() noexcept
      : pointer_(std::make_shared<std::unique_ptr<value_type>>()) {}

  DeletablePtr(const DeletablePtr& source) = default;

  DeletablePtr(DeletablePtr&& source) : pointer_(std::move(source.pointer_)) {
    source.pointer_ = std::make_shared<std::unique_ptr<value_type>>();
  }

  explicit DeletablePtr(pointer_type pointer)
      : pointer_(std::make_shared<std::unique_ptr<value_type>>(
            std::unique_ptr<value_type>(pointer))) {}

  DeletablePtr& operator=(const DeletablePtr& source) = default;

  DeletablePtr& operator=(DeletablePtr&& source) {
    Reset();
    std::swap(source.pointer_, pointer_);
    return *this;
  }

  void Reset() noexcept {
    if (*pointer_) {
      pointer_ = std::make_shared<std::unique_ptr<value_type>>();
    }
  }

  void Delete() noexcept { pointer_->reset(); }

  pointer_type Get() const { return pointer_->get(); }

  explicit operator bool() const noexcept { return Get() != nullptr; }

  value_type& operator*() const noexcept { return *Get(); }

  pointer_type operator->() const noexcept { return Get(); }

 private:
  std::shared_ptr<std::unique_ptr<value_type>> pointer_;
};

template <typename T, typename... Args>
DeletablePtr<T> MakeDeletable(Args&&... args) {
  return DeletablePtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T1, typename T2>
const bool operator==(const DeletablePtr<T1>& a,
                      const DeletablePtr<T2>& b) noexcept {
  return a.Get() == b.Get();
}

template <typename T1, typename T2>
const bool operator!=(const DeletablePtr<T1>& a,
                      const DeletablePtr<T2>& b) noexcept {
  return a.Get() != b.Get();
}

}  // namespace glight::system

#endif
