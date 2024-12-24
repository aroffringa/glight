#ifndef GLIGHT_SYSTEM_OBSERVABLE_PTR_H_
#define GLIGHT_SYSTEM_OBSERVABLE_PTR_H_

#include <cstddef>
#include <memory>

namespace glight::system {

template <typename T>
class Observer;

/**
 * An owning smart pointer that can be observed. Comparable to
 * std::weak_ptr, an observer won't stop the object from being
 * destructed. The difference with a std::shared_ptr is
 * that there can be only one unique owning pointer to an object:
 * all other pointers are observers.
 *
 * While the same behaviour can be obtained by using a single
 * std::shared_ptr with std::weak_ptrs, for cases that match with
 * the behaviour of the ObservablePtr, it offers a stricter way of
 * accessing the pointer, which may simplify structure and clarify
 * intend.
 *
 * An advantage of this class over a std::shared_ptr is that an
 * ObservablePtr never requires heap allocation and destroys the
 * object immediately when the owning pointer is destructed or reset.
 *
 * This class is not thread safe: if the owner and its observers are
 * accessed in different threads, they need to be synchronized.
 */
template <typename T>
class ObservablePtr {
 public:
  constexpr ObservablePtr() noexcept {}
  constexpr ObservablePtr(std::nullptr_t) noexcept {}
  constexpr ObservablePtr(T* object) noexcept : ptr_(object) {}
  constexpr ObservablePtr(std::unique_ptr<T> object) noexcept
      : ptr_(std::move(object)) {}
  constexpr ObservablePtr(const ObservablePtr&) noexcept = delete;
  constexpr ObservablePtr(ObservablePtr<T>&& source) noexcept;
  ~ObservablePtr() noexcept { Reset(); }

  ObservablePtr<T>& operator=(ObservablePtr<T>&& rhs) noexcept;
  ObservablePtr<T>& operator=(const ObservablePtr<T>& rhs) noexcept = delete;

  void Reset() noexcept;
  void Reset(std::nullptr_t) noexcept { Reset(); }
  void Reset(T* object) noexcept {
    Reset();
    ptr_.reset(object);
  }

  T* Get() const noexcept { return ptr_.get(); }
  constexpr T& operator*() const noexcept { return *ptr_; }
  constexpr T* operator->() const noexcept { return ptr_.get(); }

  constexpr explicit operator bool() const noexcept {
    return static_cast<bool>(ptr_);
  }
  constexpr friend bool operator==(const ObservablePtr<T>& lhs,
                                   const ObservablePtr<T>& rhs) noexcept {
    return lhs.ptr_ == rhs.ptr_;
  }
  constexpr friend bool operator!=(const ObservablePtr<T>& lhs,
                                   const ObservablePtr<T>& rhs) noexcept {
    return lhs.ptr_ != rhs.ptr_;
  }
  constexpr friend bool operator<(const ObservablePtr<T>& lhs,
                                  const ObservablePtr<T>& rhs) noexcept {
    return lhs.ptr_ < rhs.ptr_;
  }
  constexpr friend bool operator>(const ObservablePtr<T>& lhs,
                                  const ObservablePtr<T>& rhs) noexcept {
    return lhs.ptr_ > rhs.ptr_;
  }
  constexpr friend bool operator<=(const ObservablePtr<T>& lhs,
                                   const ObservablePtr<T>& rhs) noexcept {
    return lhs.ptr_ <= rhs.ptr_;
  }
  constexpr friend bool operator>=(const ObservablePtr<T>& lhs,
                                   const ObservablePtr<T>& rhs) noexcept {
    return lhs.ptr_ >= rhs.ptr_;
  }
  Observer<T> GetObserver() const noexcept;
  size_t ShareCount() const noexcept;
  T* Release() noexcept {
    T* object = ptr_.release();
    Reset();
    return object;
  }

 private:
  friend class Observer<T>;
  std::unique_ptr<T> ptr_;
  mutable Observer<T>* observer_list_ = nullptr;
};

template <typename T>
class Observer {
 public:
  Observer(const Observer& source) noexcept
      : parent_(source.parent_), previous_(nullptr), next_(nullptr) {
    if (parent_) {
      parent_->observer_list_->previous_ = this;
      next_ = parent_->observer_list_;
      parent_->observer_list_ = this;
    }
  }
  Observer(Observer&& source) noexcept
      : parent_(source.parent_),
        previous_(source.previous_),
        next_(source.next_) {
    source.parent_ = nullptr;
    source.previous_ = nullptr;
    source.next_ = nullptr;
    if (previous_)
      previous_->next_ = this;
    else if (parent_)
      parent_->observer_list_ = this;
    if (next_) next_->previous_ = this;
  }
  ~Observer() noexcept {
    if (previous_) {
      previous_->next_ = next_;
    } else if (parent_) {
      parent_->observer_list_ = next_;
    }
    if (next_) {
      next_->previous_ = previous_;
    }
  }
  Observer<T>& operator=(const Observer<T>& rhs) noexcept {
    parent_ = rhs.parent_;
    previous_ = nullptr;
    if (parent_) {
      parent_->observer_list_->previous_ = this;
      next_ = parent_->observer_list_;
      parent_->observer_list_ = this;
    } else {
      next_ = nullptr;
    }
    return *this;
  }
  Observer<T>& operator=(Observer<T>&& rhs) noexcept {
    parent_ = rhs.parent_;
    previous_ = rhs.previous_;
    next_ = rhs.next_;
    rhs.parent_ = nullptr;
    if (previous_) {
      previous_->next_ = this;
      rhs.previous_ = nullptr;
    } else if (parent_) {
      parent_->observer_list_ = this;
    }
    if (next_) {
      next_->previous_ = this;
      rhs.next_ = nullptr;
    }
    return *this;
  }
  operator bool() const { return Get() != nullptr; }
  bool operator==(const Observer& rhs) const { return Get() == rhs.Get(); }
  bool operator!=(const Observer& rhs) const { return Get() != rhs.Get(); }
  T* Get() const {
    if (parent_)
      return parent_->Get();
    else
      return nullptr;
  }

 private:
  Observer(const ObservablePtr<T>* parent, Observer<T>* previous,
           Observer<T>* next)
      : parent_(parent), previous_(previous), next_(next) {}

  friend class ObservablePtr<T>;
  const ObservablePtr<T>* parent_;
  Observer<T>* previous_;
  Observer<T>* next_;
};

template <typename T>
void ObservablePtr<T>::Reset() noexcept {
  Observer<T>* observer = observer_list_;
  while (observer) {
    Observer<T>* next = observer->next_;
    observer->parent_ = nullptr;
    observer->previous_ = nullptr;
    observer->next_ = nullptr;
    observer = next;
  }
  observer_list_ = nullptr;
  ptr_.reset();
}

template <typename T>
Observer<T> ObservablePtr<T>::GetObserver() const noexcept {
  Observer<T> observer(this, nullptr, observer_list_);
  if (observer_list_) observer_list_->previous_ = &observer;
  observer_list_ = &observer;
  return observer;
}

template <typename T>
constexpr ObservablePtr<T>::ObservablePtr(ObservablePtr<T>&& source) noexcept
    : ptr_(std::move(source.ptr_)), observer_list_(source.observer_list_) {
  source.observer_list_ = nullptr;
  Observer<T>* observer = observer_list_;
  while (observer) {
    observer->parent_ = this;
    observer = observer->next_;
  }
}

template <typename T>
ObservablePtr<T>& ObservablePtr<T>::operator=(ObservablePtr<T>&& rhs) noexcept {
  Reset();
  ptr_ = std::move(rhs.ptr_);
  observer_list_ = rhs.observer_list_;
  rhs.observer_list_ = nullptr;
  Observer<T>* observer = observer_list_;
  while (observer) {
    observer->parent_ = this;
    observer = observer->next_;
  }
  return *this;
}

template <typename T>
size_t ObservablePtr<T>::ShareCount() const noexcept {
  size_t count = 0;
  Observer<T>* observer = observer_list_;
  while (observer) {
    ++count;
    observer = observer->next_;
  }
  return count;
}

template <typename T, typename... Args>
ObservablePtr<T> MakeObservable(Args&&... args) {
  return ObservablePtr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace glight::system

#endif
