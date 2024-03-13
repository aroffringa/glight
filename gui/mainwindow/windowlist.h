#ifndef GUI_WINDOW_LIST_H_
#define GUI_WINDOW_LIST_H_

#include <gtkmm/window.h>

#include <memory>
#include <vector>

#include "theatre/forwards.h"

namespace glight::gui {

template <typename T>
class WindowList {
 public:
  void Add(std::unique_ptr<T> window) {
    T *window_ptr = window.get();
    window->signal_hide().connect(
        [&, window_ptr]() { onHideWindow(window_ptr); });
    list_.emplace_back(std::move(window));
  }

  const std::vector<std::unique_ptr<T>> &List() const { return list_; }

  T *GetOpenWindow(theatre::FolderObject &object) const {
    for (auto &window : list_) {
      if (&window->GetObject() == &object) return window.get();
    }
    return nullptr;
  }

 private:
  void onHideWindow(T *window) {
    for (auto iter = list_.begin(); iter != list_.end(); ++iter) {
      if (iter->get() == window) {
        list_.erase(iter);
        break;
      }
    }
  }

  std::vector<std::unique_ptr<T>> list_;
};

}  // namespace glight::gui

#endif
