#ifndef DEREFERENCING_ITERATOR_H_
#define DEREFERENCING_ITERATOR_H_

#include <utility>

template <typename IterT>
class DereferencingIterator {
 public:
  using value_type = decltype(**std::declval<IterT>());

  DereferencingIterator(IterT iterator) : iterator_(std::move(iterator)) {}

  IterT operator++() { return ++iterator_; }

  IterT operator--() { return --iterator_; }

  value_type& operator*() { return **iterator_; }

  bool operator==(const DereferencingIterator& rhs) const {
    return iterator_ == rhs.iterator_;
  }

  bool operator!=(const DereferencingIterator& rhs) const {
    return iterator_ != rhs.iterator_;
  }

  IterT iterator_;
};

template <typename IterT>
class DereferencingMapIterator {
 public:
  using value_type = decltype(*std::declval<IterT>()->second);

  DereferencingMapIterator(IterT iterator) : iterator_(std::move(iterator)) {}

  IterT operator++() { return ++iterator_; }

  IterT operator--() { return --iterator_; }

  value_type& operator*() { return *iterator_->second; }

  bool operator==(const DereferencingMapIterator& rhs) const {
    return iterator_ == rhs.iterator_;
  }

  bool operator!=(const DereferencingMapIterator& rhs) const {
    return iterator_ != rhs.iterator_;
  }

  IterT iterator_;
};

#endif
