#ifndef GLIGHT_SYSTEM_EVENT_SYNCHRONIZATION_H_
#define GLIGHT_SYSTEM_EVENT_SYNCHRONIZATION_H_

#include <condition_variable>
#include <mutex>

namespace glight::system {

/**
 * A synchronization class that waits for a trigger.
 * It is necessary because it behaves slightly different compared to existing
 * synchronization structures:
 * - A mutex may not be locked and unlocked from different threads.
 * - A mutex also can not be released multiple times.
 * - A barrier with n=2 wouldn't distinguish between the threads
 *   (in this class, one will always be the 'waiting' thread)
 * - A binary semaphore may not be released multiple times.
 * The use case for this class is when one thread produces some results while
 * another thread periodically consumes this. If no result is yet produced,
 * the consuming thread uses the previous produced value.
 */
class EventSynchronization {
 public:
  void Release() {
    std::unique_lock<std::mutex> lock(mutex_);
    is_released_ = true;
    condition_.notify_one();
  }

  void Wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!is_released_) {
      condition_.wait(lock);
    }
    is_released_ = false;
  }

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  bool is_released_ = false;
};

}  // namespace glight::system

#endif
