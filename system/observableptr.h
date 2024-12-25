#ifndef GLIGHT_SYSTEM_OBSERVABLE_PTR_H_
#define GLIGHT_SYSTEM_OBSERVABLE_PTR_H_

#include <cassert>
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
  constexpr explicit ObservablePtr(std::nullptr_t) noexcept {}
  /**
   * The new ObservablePtr will become the owner of the pointer and
   * make sure it is destructed.
   */
  constexpr explicit ObservablePtr(T* object) noexcept : ptr_(object) {}
  constexpr explicit ObservablePtr(std::unique_ptr<T> object) noexcept
      : ptr_(std::move(object)) {}
  /**
   * Move construct an ObservablePtr. The @p source will be reset, and all
   * observers that track the source beforehand, will track the newly
   * constructed object afterwards.
   */
  constexpr ObservablePtr(ObservablePtr<T>&& source) noexcept;
  constexpr ~ObservablePtr() noexcept { Reset(); }

  /**
   * Move assign an ObservablePtr. The @p rhs will be reset, and all observers
   * that track the rhs beforehand, will track the destination object
   * afterwards.
   */
  ObservablePtr<T>& operator=(ObservablePtr<T>&& rhs) noexcept;

  constexpr ObservablePtr(const ObservablePtr&) noexcept = delete;
  constexpr ObservablePtr<T>& operator=(const ObservablePtr<T>& rhs) noexcept =
      delete;

  /**
   * If set, destroys the object being pointed to and unlinks
   * all the observers. This means that if this pointer is
   * later set again, it won't affect the current observers:
   * their Get() function will still return @c nullptr.
   */
  void Reset() noexcept;
  /** Same as @ref Reset(). */
  void Reset(std::nullptr_t) noexcept { Reset(); }
  /**
   * Resets the objects and assigns the object afterwards.
   * Any observers will not be tracking the new object. This
   * ObservablePtr will become the owner of the pointer and
   * make sure it is destructed.
   */
  void Reset(T* object) noexcept {
    Reset();
    ptr_.reset(object);
  }
  void Reset(std::unique_ptr<T> object_ptr) noexcept {
    Reset();
    ptr_ = std::move(object_ptr);
  }

  T* Get() const noexcept { return ptr_.get(); }
  constexpr T& operator*() const noexcept {
    assert(ptr_);
    return *ptr_;
  }
  constexpr T* operator->() const noexcept {
    assert(ptr_);
    return ptr_.get();
  }

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

  /**
   * Number of observers that track this pointer.
   */
  size_t ShareCount() const noexcept;

  /**
   * Transfers ownership of the pointer. After this call, this
   * ObservablePtr will be in a reset state.
   */
  std::unique_ptr<T> Release() noexcept {
    std::unique_ptr<T> object = std::move(ptr_);
    Reset();
    return object;
  }

  friend void swap(ObservablePtr<T>& a, ObservablePtr<T>& b) {
    std::swap(a.ptr_, b.ptr_);
    std::swap(a.observer_list_, b.observer_list_);
    const auto set_list = [](Observer<T>* list, ObservablePtr<T>& new_parent) {
      while (list) {
        list->parent_ = &new_parent;
        list = list->next_;
      }
    };
    set_list(a.observer_list_, a);
    set_list(b.observer_list_, b);
  }

 private:
  friend class Observer<T>;
  std::unique_ptr<T> ptr_;
  mutable Observer<T>* observer_list_ = nullptr;
};

template <typename T>
class Observer {
 public:
  constexpr Observer() noexcept
      : parent_(nullptr), previous_(nullptr), next_(nullptr) {}
  constexpr Observer(std::nullptr_t) noexcept : Observer() {}
  constexpr Observer(const Observer& source) noexcept
      : parent_(source.parent_), previous_(nullptr), next_(nullptr) {
    if (parent_) {
      parent_->observer_list_->previous_ = this;
      next_ = parent_->observer_list_;
      parent_->observer_list_ = this;
    }
  }
  constexpr Observer(Observer&& source) noexcept
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
  constexpr ~Observer() noexcept {
    if (previous_) {
      previous_->next_ = next_;
    } else if (parent_) {
      parent_->observer_list_ = next_;
    }
    if (next_) {
      next_->previous_ = previous_;
    }
  }
  constexpr Observer<T>& operator=(const Observer<T>& rhs) noexcept {
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
  constexpr Observer<T>& operator=(Observer<T>&& rhs) noexcept {
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
  constexpr operator bool() const { return Get() != nullptr; }
  constexpr bool operator==(const Observer& rhs) const {
    return Get() == rhs.Get();
  }
  constexpr bool operator!=(const Observer& rhs) const {
    return Get() != rhs.Get();
  }
  constexpr T* Get() const {
    if (parent_)
      return parent_->Get();
    else
      return nullptr;
  }

  constexpr friend void swap(Observer<T>& a, Observer<T>& b) {
    std::swap(a.parent_, b.parent_);
    if (a.parent_ != b.parent_) {
      std::swap(a.previous_, b.previous_);
      std::swap(a.next_, b.next_);
      if (a.next_) a.next_->previous_ = &a;
      if (a.previous_)
        a.previous_->next_ = &a;
      else if (a.parent_)
        a.parent_->observer_list_ = &a;

      if (b.next_) b.next_->previous_ = &b;
      if (b.previous_)
        b.previous_->next_ = &b;
      else if (b.parent_)
        b.parent_->observer_list_ = &b;
    }
  }

 private:
  constexpr Observer(const ObservablePtr<T>* parent, Observer<T>* previous,
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
