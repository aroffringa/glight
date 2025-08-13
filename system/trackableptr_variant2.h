#ifndef GLIGHT_SYSTEM_OBSERVABLE_PTR_H_
#define GLIGHT_SYSTEM_OBSERVABLE_PTR_H_

#include <cassert>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace glight::system {

template <typename T>
class ObservingPtr;

namespace internal {

struct TrackablePtrData {
  void* object = nullptr;
  size_t reference_count = 0;
};

template <typename From, typename To>
concept StaticConversion = requires(From f) { static_cast<To>(f); };

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
class TrackablePtr {
 public:
  constexpr TrackablePtr() noexcept = default;
  constexpr explicit TrackablePtr(std::nullptr_t) noexcept {};

  /**
   * The new TrackablePtr will become the owner of the pointer and
   * make sure it is destructed.
   */
  constexpr explicit TrackablePtr(T* object) noexcept : object_(object) {}
  constexpr explicit TrackablePtr(std::unique_ptr<T> object) noexcept
      : object_(object.release()) {}
  /**
   * Move construct a TrackablePtr. The @p source will be reset, and all
   * observers that track the source beforehand, will track the newly
   * constructed object afterwards.
   */
  constexpr TrackablePtr(TrackablePtr<T>&& source) noexcept
      : object_(source.object_), data_(source.data_) {
    source.object_ = nullptr;
    source.data_ = nullptr;
  }
  constexpr ~TrackablePtr() noexcept {
    if (data_) {
      --data_->reference_count;
      if (data_->reference_count == 0) {
        delete data_;
      } else {
        data_->object = nullptr;
      }
    }
    delete object_;
  }

  /**
   * Move assign an TrackablePtr. The @p rhs will be reset, and all observers
   * that track the rhs beforehand, will track the destination object
   * afterwards.
   */
  TrackablePtr<T>& operator=(TrackablePtr<T>&& rhs) noexcept {
    if (&rhs != this) {
      delete object_;
      if (data_) {
        --data_->reference_count;
        if (data_->reference_count == 0) {
          delete data_;
        } else {
          data_->object = nullptr;
        }
      }
      object_ = rhs.object_;
      data_ = rhs.data_;
      rhs.object_ = nullptr;
      rhs.data_ = nullptr;
    }
    return *this;
  }

  constexpr TrackablePtr(const TrackablePtr&) noexcept = delete;
  constexpr TrackablePtr<T>& operator=(const TrackablePtr<T>& rhs) noexcept =
      delete;

  /**
   * If set, destroys the object being pointed to and unlinks
   * all the observers. This means that if this pointer is
   * later set again, it won't affect the current observers:
   * their Get() function will still return @c nullptr.
   */
  void Reset() noexcept {
    delete object_;
    object_ = nullptr;
    if (data_) {
      --data_->reference_count;
      if (data_->reference_count == 0) {
        delete data_;
      } else {
        data_->object = nullptr;
      }
      data_ = nullptr;
    }
  }
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
    object_ = object;
  }
  void Reset(std::unique_ptr<T> object_ptr) noexcept {
    Reset();
    object_ = object_ptr.release();
  }

  T* Get() const noexcept { return object_; }
  constexpr T& operator*() const noexcept {
    assert(object_);
    return *object_;
  }
  constexpr T* operator->() const noexcept {
    assert(object_);
    return object_;
  }

  constexpr explicit operator bool() const noexcept {
    return static_cast<bool>(object_);
  }

  constexpr friend bool operator==(const TrackablePtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.object_ == rhs.object_;
  }
  constexpr friend bool operator!=(const TrackablePtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.object_ != rhs.object_;
  }
  constexpr friend bool operator<(const TrackablePtr<T>& lhs,
                                  const TrackablePtr<T>& rhs) noexcept {
    return lhs.object_ < rhs.object_;
  }
  constexpr friend bool operator>(const TrackablePtr<T>& lhs,
                                  const TrackablePtr<T>& rhs) noexcept {
    return lhs.object_ > rhs.object_;
  }
  constexpr friend bool operator<=(const TrackablePtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.object_ <= rhs.object_;
  }
  constexpr friend bool operator>=(const TrackablePtr<T>& lhs,
                                   const TrackablePtr<T>& rhs) noexcept {
    return lhs.object_ >= rhs.object_;
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
  requires(std::is_same_v<ObserverType, T> || std::is_convertible_v<ObserverType, T>)
  ObservingPtr<ObserverType> GetObserver() const;

  /**
   * Number of observers that track this pointer.
   */
  size_t ShareCount() const noexcept {
    if (data_) {
      return data_->reference_count - 1;
    } else {
      return 0;
    }
  }

  /**
   * Transfers ownership of the pointer. After this call, this
   * TrackablePtr will be in a reset state.
   */
  std::unique_ptr<T> Release() noexcept {
    T* object = object_;
    object_ = nullptr;
    if (data_) {
      --data_->reference_count;
      if (data_->reference_count == 0) {
        delete data_;
      } else {
        data_->object = nullptr;
      }
      data_ = nullptr;
    }
    return std::unique_ptr<T>(object);
  }

  friend void swap(TrackablePtr<T>& a, TrackablePtr<T>& b) {
    std::swap(a.object_, b.object_);
    std::swap(a.data_, b.data_);
  }

 private:
  T* object_ = nullptr;
  mutable internal::TrackablePtrData* data_ = nullptr;
};

/**
 * A class that can observe a TrackablePtr. An ObservingPtr will not
 * extend the lifetime of a pointer, similar to a std::weak_ptr. An
 * ObservingPtr keeps track whether the object in a TrackablePtr is destructed.
 * Once the object is destructed, the ObservingPtr is equivalent to nullptr
 * and will no longer track the TrackablePtr.
 */
template <typename T>
class ObservingPtr {
 public:
  constexpr ObservingPtr() noexcept = default;
  constexpr ObservingPtr(std::nullptr_t) noexcept {}
  /**
   * Copy construct an ObservingPtr. The new ObservingPtr will track
   * the same pointer as the @p source.
   */
  constexpr ObservingPtr(const ObservingPtr& source) noexcept
      : data_(source.data_) {
    if (data_) {
      ++data_->reference_count;
    }
  }
  /**
   * Move construct an ObservingPtr. The @p source will be
   * equivalent to nullptr after the move.
   */
  constexpr ObservingPtr(ObservingPtr&& source) noexcept : data_(source.data_) {
    source.data_ = nullptr;
  }
  constexpr ~ObservingPtr() noexcept {
    if (data_) {
      --data_->reference_count;
      if (data_->reference_count == 0) delete data_;
    }
  }
  ObservingPtr<T>& operator=(const ObservingPtr<T>& rhs) noexcept {
    // For the case of self-assignment, the counter needs to be increased first
    if (rhs.data_) {
      ++rhs.data_->reference_count;
    }
    if (data_) {
      --data_->reference_count;
      if (data_->reference_count == 0) delete data_;
    }
    data_ = rhs.data_;
    return *this;
  }
  ObservingPtr<T>& operator=(ObservingPtr<T>&& rhs) noexcept {
    if (&rhs != this) {
      if (data_) {
        --data_->reference_count;
        if (data_->reference_count == 0) delete data_;
      }
      data_ = rhs.data_;
      rhs.data_ = nullptr;
    }
    return *this;
  }
  constexpr operator bool() const { return Get() != nullptr; }

  template <typename ImplicitCastableType>
  requires(std::is_convertible_v<T*, ImplicitCastableType*>)
  constexpr operator ObservingPtr<ImplicitCastableType>() const & {
    if (data_) data_->reference_count++;
    return ObservingPtr<ImplicitCastableType>(data_);
  }

  template <typename ImplicitCastableType>
  requires(std::is_convertible_v<T*, ImplicitCastableType*>)
  constexpr operator ObservingPtr<ImplicitCastableType>() && {
    ObservingPtr<ImplicitCastableType> result(data_);
    data_ = nullptr;
    return result;
  }

  template <typename ExplicitCastableType>
  requires(!std::is_convertible_v<T*, ExplicitCastableType*> && internal::StaticConversion<T*, ExplicitCastableType*>)
  constexpr explicit operator ObservingPtr<ExplicitCastableType>() const & {
    if (data_) data_->reference_count++;
    return ObservingPtr<ExplicitCastableType>(data_);
  }

  template <typename ExplicitCastableType>
  requires(!std::is_convertible_v<T*, ExplicitCastableType*> && internal::StaticConversion<T*, ExplicitCastableType*>)
  constexpr explicit operator ObservingPtr<ExplicitCastableType>() && {
    ObservingPtr<ExplicitCastableType> result(data_);
    data_ = nullptr;
    return result;
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
    return data_ ? static_cast<T*>(data_->object) : nullptr;
  }
  constexpr T& operator*() const noexcept {
    assert(data_);
    assert(data_->object);
    return *static_cast<T*>(data_->object);
  }
  constexpr T* operator->() const noexcept {
    assert(data_);
    assert(data_->object);
    return static_cast<T*>(data_->object);
  }

  constexpr friend void swap(ObservingPtr<T>& a, ObservingPtr<T>& b) {
    std::swap(a.data_, b.data_);
  }

 private:
  constexpr ObservingPtr(internal::TrackablePtrData* data) : data_(data) {}
  template <typename S>
  friend class TrackablePtr;
  template <typename S>
  friend class ObservingPtr;
  internal::TrackablePtrData* data_ = nullptr;
};

template <typename T>
template <typename ObservingType>
requires(std::is_same_v<ObservingType, T> || std::is_convertible_v<ObservingType, T>)
ObservingPtr<ObservingType> TrackablePtr<T>::GetObserver() const {
  if (data_ == nullptr) {
    data_ =
        new internal::TrackablePtrData{.object = object_, .reference_count = 2};
  } else {
    data_->reference_count++;
  }
  return ObservingPtr<ObservingType>(data_);
}

template <typename T, typename... Args>
TrackablePtr<T> MakeTrackable(Args&&... args) {
  return TrackablePtr<T>(new T(std::forward<Args>(args)...));
}

template<typename To, typename From>
ObservingPtr<To> StaticObserverCast(const ObservingPtr<From>& from) {
  return static_cast<ObservingPtr<To>>(from);
}

template<typename To, typename From>
ObservingPtr<To> StaticObserverCast(ObservingPtr<From>&& from) {
  return static_cast<ObservingPtr<To>>(std::move(from));
}

}  // namespace glight::system

#endif
