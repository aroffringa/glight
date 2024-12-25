#ifndef GLIGHT_SYSTEM_OBSERVABLE_PTR_H_
#define GLIGHT_SYSTEM_OBSERVABLE_PTR_H_

#include <cassert>
#include <cstddef>
#include <memory>

namespace glight::system {

template <typename T>
class ObservingPtr;

/**
 * An owning smart pointer that can be observed. Comparable to
 * std::weak_ptr, an observer won't stop the object from being
 * destructed. The difference with a std::shared_ptr is
 * that there can be only one unique owning pointer to an object:
 * all other pointers are observers.
 *
 * While the same behaviour can be obtained by using a single
 * std::shared_ptr with std::weak_ptrs, for cases that match with
 * the behaviour of the TrackablePtr, it offers a stricter way of
 * accessing the pointer, which may simplify structure and clarify
 * intend.
 *
 * An advantage of this class over a std::shared_ptr is that an
 * TrackablePtr never requires heap allocation and destroys the
 * object immediately when the owning pointer is destructed or reset.
 *
 * This class is not thread safe: if the owner and its observers are
 * accessed in different threads, they need to be synchronized.
 */
template <typename T>
class TrackablePtr {
 public:
  constexpr TrackablePtr() noexcept {}
  constexpr explicit TrackablePtr(std::nullptr_t) noexcept {}
  /**
   * The new TrackablePtr will become the owner of the pointer and
   * make sure it is destructed.
   */
  constexpr explicit TrackablePtr(T* object) noexcept : ptr_(object) {}
  constexpr explicit TrackablePtr(std::unique_ptr<T> object) noexcept
      : ptr_(std::move(object)) {}
  /**
   * Move construct an TrackablePtr. The @p source will be reset, and all
   * observers that track the source beforehand, will track the newly
   * constructed object afterwards.
   */
  constexpr TrackablePtr(TrackablePtr<T>&& source) noexcept;
  constexpr ~TrackablePtr() noexcept { Reset(); }

  /**
   * Move assign an TrackablePtr. The @p rhs will be reset, and all observers
   * that track the rhs beforehand, will track the destination object
   * afterwards.
   */
  TrackablePtr<T>& operator=(TrackablePtr<T>&& rhs) noexcept;

  constexpr TrackablePtr(const TrackablePtr&) noexcept = delete;
  constexpr TrackablePtr<T>& operator=(const TrackablePtr<T>& rhs) noexcept =
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
   * TrackablePtr will become the owner of the pointer and
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
  constexpr friend bool operator==(const TrackablePtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.ptr_ == rhs.ptr_;
  }
  constexpr friend bool operator!=(const TrackablePtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.ptr_ != rhs.ptr_;
  }
  constexpr friend bool operator<(const TrackablePtr<T>& lhs,
                                  const TrackablePtr<T>& rhs) noexcept {
    return lhs.ptr_ < rhs.ptr_;
  }
  constexpr friend bool operator>(const TrackablePtr<T>& lhs,
                                  const TrackablePtr<T>& rhs) noexcept {
    return lhs.ptr_ > rhs.ptr_;
  }
  constexpr friend bool operator<=(const TrackablePtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.ptr_ <= rhs.ptr_;
  }
  constexpr friend bool operator>=(const TrackablePtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.ptr_ >= rhs.ptr_;
  }
  ObservingPtr<T> GetObserver() const noexcept;

  /**
   * Number of observers that track this pointer.
   */
  size_t ShareCount() const noexcept;

  /**
   * Transfers ownership of the pointer. After this call, this
   * TrackablePtr will be in a reset state.
   */
  std::unique_ptr<T> Release() noexcept {
    std::unique_ptr<T> object = std::move(ptr_);
    Reset();
    return object;
  }

  friend void swap(TrackablePtr<T>& a, TrackablePtr<T>& b) {
    std::swap(a.ptr_, b.ptr_);
    std::swap(a.observer_list_, b.observer_list_);
    const auto set_list = [](ObservingPtr<T>* list,
                             TrackablePtr<T>& new_parent) {
      while (list) {
        list->parent_ = &new_parent;
        list = list->next_;
      }
    };
    set_list(a.observer_list_, a);
    set_list(b.observer_list_, b);
  }

 private:
  friend class ObservingPtr<T>;
  std::unique_ptr<T> ptr_;
  mutable ObservingPtr<T>* observer_list_ = nullptr;
};

template <typename T>
class ObservingPtr {
 public:
  constexpr ObservingPtr() noexcept
      : parent_(nullptr), previous_(nullptr), next_(nullptr) {}
  constexpr ObservingPtr(std::nullptr_t) noexcept : ObservingPtr() {}
  constexpr ObservingPtr(const ObservingPtr& source) noexcept
      : parent_(source.parent_), previous_(nullptr), next_(nullptr) {
    if (parent_) {
      parent_->observer_list_->previous_ = this;
      next_ = parent_->observer_list_;
      parent_->observer_list_ = this;
    }
  }
  constexpr ObservingPtr(ObservingPtr&& source) noexcept
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
  constexpr ~ObservingPtr() noexcept {
    if (previous_) {
      previous_->next_ = next_;
    } else if (parent_) {
      parent_->observer_list_ = next_;
    }
    if (next_) {
      next_->previous_ = previous_;
    }
  }
  constexpr ObservingPtr<T>& operator=(const ObservingPtr<T>& rhs) noexcept {
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
  constexpr ObservingPtr<T>& operator=(ObservingPtr<T>&& rhs) noexcept {
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
  constexpr bool operator==(const ObservingPtr& rhs) const {
    return Get() == rhs.Get();
  }
  constexpr bool operator!=(const ObservingPtr& rhs) const {
    return Get() != rhs.Get();
  }
  constexpr T* Get() const {
    if (parent_)
      return parent_->Get();
    else
      return nullptr;
  }

  constexpr friend void swap(ObservingPtr<T>& a, ObservingPtr<T>& b) {
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
  constexpr ObservingPtr(const TrackablePtr<T>* parent,
                         ObservingPtr<T>* previous, ObservingPtr<T>* next)
      : parent_(parent), previous_(previous), next_(next) {}

  friend class TrackablePtr<T>;
  const TrackablePtr<T>* parent_;
  ObservingPtr<T>* previous_;
  ObservingPtr<T>* next_;
};

template <typename T>
void TrackablePtr<T>::Reset() noexcept {
  ObservingPtr<T>* observer = observer_list_;
  while (observer) {
    ObservingPtr<T>* next = observer->next_;
    observer->parent_ = nullptr;
    observer->previous_ = nullptr;
    observer->next_ = nullptr;
    observer = next;
  }
  observer_list_ = nullptr;
  ptr_.reset();
}

template <typename T>
ObservingPtr<T> TrackablePtr<T>::GetObserver() const noexcept {
  ObservingPtr<T> observer(this, nullptr, observer_list_);
  if (observer_list_) observer_list_->previous_ = &observer;
  observer_list_ = &observer;
  return observer;
}

template <typename T>
constexpr TrackablePtr<T>::TrackablePtr(TrackablePtr<T>&& source) noexcept
    : ptr_(std::move(source.ptr_)), observer_list_(source.observer_list_) {
  source.observer_list_ = nullptr;
  ObservingPtr<T>* observer = observer_list_;
  while (observer) {
    observer->parent_ = this;
    observer = observer->next_;
  }
}

template <typename T>
TrackablePtr<T>& TrackablePtr<T>::operator=(TrackablePtr<T>&& rhs) noexcept {
  Reset();
  ptr_ = std::move(rhs.ptr_);
  observer_list_ = rhs.observer_list_;
  rhs.observer_list_ = nullptr;
  ObservingPtr<T>* observer = observer_list_;
  while (observer) {
    observer->parent_ = this;
    observer = observer->next_;
  }
  return *this;
}

template <typename T>
size_t TrackablePtr<T>::ShareCount() const noexcept {
  size_t count = 0;
  ObservingPtr<T>* observer = observer_list_;
  while (observer) {
    ++count;
    observer = observer->next_;
  }
  return count;
}

template <typename T, typename... Args>
TrackablePtr<T> MakeObservable(Args&&... args) {
  return TrackablePtr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace glight::system

#endif
