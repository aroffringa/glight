#ifndef GLIGHT_SYSTEM_OBSERVABLE_PTR_H_
#define GLIGHT_SYSTEM_OBSERVABLE_PTR_H_

#include <cassert>
#include <cstddef>
#include <memory>

namespace glight::system {

template <typename T>
class ObservingPtr;

namespace internal {

struct ObservingPtrData;

struct TrackablePtrData {
  constexpr TrackablePtrData(void* ptr,
                             ObservingPtrData* list_head = nullptr) noexcept
      : ptr_(ptr), observer_list_(list_head) {}
  void* ptr_;
  mutable ObservingPtrData* observer_list_;

  template <typename ObserverType>
  ObservingPtr<ObserverType> GetObserver() const noexcept;
  friend struct ObservingPtrData;
  void SetParent(ObservingPtrData* observer) const;
};

struct ObservingPtrData {
  constexpr ObservingPtrData(const TrackablePtrData* parent,
                             ObservingPtrData* previous,
                             ObservingPtrData* next) noexcept
      : parent_(parent), previous_(previous), next_(next) {}
  const TrackablePtrData* parent_ = nullptr;
  ObservingPtrData* previous_ = nullptr;
  ObservingPtrData* next_ = nullptr;
  friend class TrackingPtrData;
};

inline void TrackablePtrData::SetParent(ObservingPtrData* observer) const {
  // This function is necessary to avoid access problems from friend functions
  // into a private member of ObservingPtr, which some compilers don't
  // support.'
  observer->parent_ = this;
}

}  // namespace internal

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
class TrackablePtr : private internal::TrackablePtrData {
 public:
  constexpr TrackablePtr() noexcept : TrackablePtrData(nullptr) {}
  constexpr explicit TrackablePtr(std::nullptr_t) noexcept
      : TrackablePtrData(nullptr) {}
  /**
   * The new TrackablePtr will become the owner of the pointer and
   * make sure it is destructed.
   */
  constexpr explicit TrackablePtr(T* object) noexcept
      : TrackablePtrData(object) {}
  constexpr explicit TrackablePtr(std::unique_ptr<T> object) noexcept
      : TrackablePtrData(object.release()) {}
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
    delete static_cast<T*>(ptr_);
    ptr_ = object;
  }
  void Reset(std::unique_ptr<T> object_ptr) noexcept {
    Reset();
    ptr_ = object_ptr.release();
  }

  T* Get() const noexcept { return static_cast<T*>(ptr_); }
  constexpr T& operator*() const noexcept {
    assert(ptr_);
    return *static_cast<T*>(ptr_);
  }
  constexpr T* operator->() const noexcept {
    assert(ptr_);
    return static_cast<T*>(ptr_);
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

  constexpr friend bool operator==(const TrackablePtr<T>& lhs,
                                   const ObservingPtr<T>& rhs) noexcept {
    return lhs.Get() == rhs.Get();
  }
  constexpr friend bool operator!=(const TrackablePtr<T>& lhs,
                                   const ObservingPtr<T>& rhs) noexcept {
    return lhs.Get() != rhs.Get();
  }
  constexpr friend bool operator<(const TrackablePtr<T>& lhs,
                                  const ObservingPtr<T>& rhs) noexcept {
    return lhs.Get() < rhs.Get();
  }
  constexpr friend bool operator>(const TrackablePtr<T>& lhs,
                                  const ObservingPtr<T>& rhs) noexcept {
    return lhs.Get() > rhs.Get();
  }
  constexpr friend bool operator<=(const TrackablePtr<T>& lhs,
                                   const ObservingPtr<T>& rhs) noexcept {
    return lhs.Get() <= rhs.Get();
  }
  constexpr friend bool operator>=(const TrackablePtr<T>& lhs,
                                   const ObservingPtr<T>& rhs) noexcept {
    return lhs.Get() >= rhs.Get();
  }

  constexpr friend bool operator==(const ObservingPtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.Get() == rhs.Get();
  }
  constexpr friend bool operator!=(const ObservingPtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.Get() != rhs.Get();
  }
  constexpr friend bool operator<(const ObservingPtr<T>& lhs,
                                  const TrackablePtr<T>& rhs) noexcept {
    return lhs.Get() < rhs.Get();
  }
  constexpr friend bool operator>(const ObservingPtr<T>& lhs,
                                  const TrackablePtr<T>& rhs) noexcept {
    return lhs.Get() > rhs.Get();
  }
  constexpr friend bool operator<=(const ObservingPtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.Get() <= rhs.Get();
  }
  constexpr friend bool operator>=(const ObservingPtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.Get() >= rhs.Get();
  }

  template <typename ObserverType = T>
  ObservingPtr<ObserverType> GetObserver() const noexcept {
    return TrackablePtrData::GetObserver<ObserverType>();
  }

  /**
   * Number of observers that track this pointer.
   */
  size_t ShareCount() const noexcept;

  /**
   * Transfers ownership of the pointer. After this call, this
   * TrackablePtr will be in a reset state.
   */
  std::unique_ptr<T> Release() noexcept {
    std::unique_ptr<T> object(static_cast<T*>(ptr_));
    ptr_ = nullptr;
    Reset();
    return object;
  }

  friend void swap(TrackablePtr<T>& a, TrackablePtr<T>& b) {
    std::swap(a.ptr_, b.ptr_);
    std::swap(a.observer_list_, b.observer_list_);
    const auto set_list = [](internal::ObservingPtrData* list,
                             internal::TrackablePtrData& new_parent) {
      while (list) {
        new_parent.SetParent(list);
        list = Next(*list);
      }
    };
    set_list(a.observer_list_, a);
    set_list(b.observer_list_, b);
  }

 private:
  friend class internal::ObservingPtrData;

  static internal::ObservingPtrData* Next(internal::ObservingPtrData& p) {
    return p.next_;
  }
};

/**
 * A class that can observe a TrackablePtr. An ObservingPtr will not
 * extend the lifetime of a pointer, similar to a std::weak_ptr. An
 * ObservingPtr keeps track whether the object in a TrackablePtr is destructed.
 * Once the object is destructed, the ObservingPtr is equivalent to nullptr
 * and will no longer track the TrackablePtr.
 */
template <typename T>
class ObservingPtr : private internal::ObservingPtrData {
 public:
  constexpr ObservingPtr() noexcept
      : ObservingPtrData(nullptr, nullptr, nullptr) {}
  constexpr ObservingPtr(std::nullptr_t) noexcept
      : ObservingPtrData(nullptr, nullptr, nullptr) {}
  /**
   * Copy construct an ObservingPtr. The new ObservingPtr will track
   * the same pointer as the @p source.
   */
  constexpr ObservingPtr(const ObservingPtr& source) noexcept
      : ObservingPtrData(source.parent_, nullptr, nullptr) {
    if (parent_) {
      parent_->observer_list_->previous_ = this;
      next_ = parent_->observer_list_;
      parent_->observer_list_ = this;
    }
  }
  /**
   * Move construct an ObservingPtr. The @p source will be
   * equivalent to nullptr after the move.
   */
  constexpr ObservingPtr(ObservingPtr&& source) noexcept
      : ObservingPtrData(source.parent_, source.previous_, source.next_) {
    source.parent_ = nullptr;
    if (previous_) {
      previous_->next_ = this;
      source.previous_ = nullptr;
    } else if (parent_) {
      parent_->observer_list_ = this;
    }
    if (next_) {
      next_->previous_ = this;
      source.next_ = nullptr;
    }
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
    // First remove this ObservingPtr from the linked list
    if (previous_) {
      previous_->next_ = next_;
    } else if (parent_) {
      parent_->observer_list_ = next_;
    }
    if (next_) {
      next_->previous_ = previous_;
    }
    parent_ = rhs.parent_;
    // Insert in linked list
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
    // If the lhs and rhs have the same parent, we do not have to change
    // this position in the linked list, and only remove the rhs from the
    // linked list.
    if (parent_ == rhs.parent_) {
      if (this == &rhs) return *this;
      if (rhs.previous_) {
        rhs.previous_->next_ = rhs.next_;
      } else if (rhs.parent_) {
        rhs.parent_->observer_list_ = rhs.next_;
      }
      if (rhs.next_) {
        rhs.next_->previous_ = rhs.previous_;
        rhs.next_ = nullptr;
      }
      rhs.parent_ = nullptr;
      rhs.previous_ = nullptr;
    } else {
      // First remove this ObservingPtr from the linked list
      if (previous_) {
        previous_->next_ = next_;
      } else if (parent_) {
        parent_->observer_list_ = next_;
      }
      if (next_) {
        next_->previous_ = previous_;
      }
      // Take position of rhs
      parent_ = rhs.parent_;
      previous_ = rhs.previous_;
      next_ = rhs.next_;
      // Update linked list
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
    }
    return *this;
  }
  constexpr operator bool() const { return Get() != nullptr; }
  template <typename ImplicitCastableType>
  constexpr operator ObservingPtr<ImplicitCastableType>() const {
    if (parent_)
      return parent_->template GetObserver<ImplicitCastableType>();
    else
      return ObservingPtr<ImplicitCastableType>();
  }
  constexpr bool operator==(const ObservingPtr& rhs) const {
    return Get() == rhs.Get();
  }
  constexpr bool operator!=(const ObservingPtr& rhs) const {
    return Get() != rhs.Get();
  }
  constexpr bool operator<(const ObservingPtr& rhs) const {
    return Get() < rhs.Get();
  }
  constexpr bool operator>(const ObservingPtr& rhs) const {
    return Get() > rhs.Get();
  }
  constexpr bool operator<=(const ObservingPtr& rhs) const {
    return Get() <= rhs.Get();
  }
  constexpr bool operator>=(const ObservingPtr& rhs) const {
    return Get() >= rhs.Get();
  }
  constexpr friend bool operator==(const ObservingPtr& lhs, const T* rhs) {
    return lhs.Get() == rhs;
  }
  constexpr friend bool operator==(const T* lhs, const ObservingPtr& rhs) {
    return lhs == rhs.Get();
  }
  constexpr friend bool operator!=(const ObservingPtr& lhs, const T* rhs) {
    return lhs.Get() != rhs;
  }
  constexpr friend bool operator!=(const T* lhs, const ObservingPtr& rhs) {
    return lhs != rhs.Get();
  }
  constexpr friend bool operator<(const ObservingPtr& lhs, const T* rhs) {
    return lhs.Get() < rhs;
  }
  constexpr friend bool operator<(const T* lhs, const ObservingPtr& rhs) {
    return lhs < rhs.Get();
  }
  constexpr friend bool operator>(const ObservingPtr& lhs, const T* rhs) {
    return lhs.Get() > rhs;
  }
  constexpr friend bool operator>(const T* lhs, const ObservingPtr& rhs) {
    return lhs > rhs.Get();
  }
  constexpr friend bool operator<=(const ObservingPtr& lhs, const T* rhs) {
    return lhs.Get() <= rhs;
  }
  constexpr friend bool operator<=(const T* lhs, const ObservingPtr& rhs) {
    return lhs <= rhs.Get();
  }
  constexpr friend bool operator>=(const ObservingPtr& lhs, const T* rhs) {
    return lhs.Get() >= rhs;
  }
  constexpr friend bool operator>=(const T* lhs, const ObservingPtr& rhs) {
    return lhs >= rhs.Get();
  }
  constexpr T* Get() const {
    return parent_ ? static_cast<T*>(parent_->ptr_) : nullptr;
  }
  constexpr T& operator*() const noexcept {
    assert(parent_);
    assert(parent_->ptr_);
    return *static_cast<T*>(parent_->ptr_);
  }
  constexpr T* operator->() const noexcept {
    assert(parent_);
    assert(parent_->ptr_);
    return static_cast<T*>(parent_->ptr_);
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
        a.SetParentObserverList(a);

      if (b.next_) b.next_->previous_ = &b;
      if (b.previous_)
        b.previous_->next_ = &b;
      else if (b.parent_)
        b.SetParentObserverList(b);
    }
  }

 private:
  friend struct internal::TrackablePtrData;
  constexpr ObservingPtr(const internal::TrackablePtrData* parent,
                         internal::ObservingPtrData* previous,
                         internal::ObservingPtrData* next) noexcept
      : ObservingPtrData(parent, previous, next) {}

  void SetParentObserverList(ObservingPtr<T>& new_head) const {
    // This function is necessary to avoid access problems from friend functions
    // into a private member of TrackablePtr, which some compilers don't
    // support.'
    parent_->observer_list_ = &new_head;
  }
};

template <typename T>
void TrackablePtr<T>::Reset() noexcept {
  internal::ObservingPtrData* observer = observer_list_;
  while (observer) {
    internal::ObservingPtrData* next = observer->next_;
    observer->parent_ = nullptr;
    observer->previous_ = nullptr;
    observer->next_ = nullptr;
    observer = next;
  }
  observer_list_ = nullptr;
  delete static_cast<T*>(ptr_);
  ptr_ = nullptr;
}

template <typename ObserverType>
ObservingPtr<ObserverType> internal::TrackablePtrData::GetObserver()
    const noexcept {
  ObservingPtr<ObserverType> observer(this, nullptr, observer_list_);
  if (observer_list_) observer_list_->previous_ = &observer;
  observer_list_ = &observer;
  return observer;
}

template <typename T>
constexpr TrackablePtr<T>::TrackablePtr(TrackablePtr<T>&& source) noexcept
    : TrackablePtrData(source.ptr_, source.observer_list_) {
  source.ptr_ = nullptr;
  source.observer_list_ = nullptr;
  internal::ObservingPtrData* observer = observer_list_;
  while (observer) {
    observer->parent_ = this;
    observer = observer->next_;
  }
}

template <typename T>
TrackablePtr<T>& TrackablePtr<T>::operator=(TrackablePtr<T>&& rhs) noexcept {
  if (this == &rhs) return *this;
  Reset();
  ptr_ = rhs.ptr_;
  observer_list_ = rhs.observer_list_;
  rhs.ptr_ = nullptr;
  rhs.observer_list_ = nullptr;
  internal::ObservingPtrData* observer = observer_list_;
  while (observer) {
    observer->parent_ = this;
    observer = observer->next_;
  }
  return *this;
}

template <typename T>
size_t TrackablePtr<T>::ShareCount() const noexcept {
  size_t count = 0;
  internal::ObservingPtrData* observer = observer_list_;
  while (observer) {
    ++count;
    observer = observer->next_;
  }
  return count;
}

template <typename T, typename... Args>
TrackablePtr<T> MakeTrackable(Args&&... args) {
  return TrackablePtr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace glight::system

#endif
