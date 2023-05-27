#ifndef GLIGHT_SYSTEM_EVENT_SYNCHRONIZATION_H_
#define GLIGHT_SYSTEM_EVENT_SYNCHRONIZATION_H_

#include <condition_variable>
#include <mutex>

namespace glight::system {

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
