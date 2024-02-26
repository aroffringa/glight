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
    window->signal_hide().connect([&]() { onHideWindow(window.get()); });
    _list.emplace_back(std::move(window));
  }

  const std::vector<std::unique_ptr<T>> &List() const { return _list; }

  T *GetOpenWindow(theatre::FolderObject &object) const {
    for (auto &window : _list) {
      if (&window->GetObject() == &object) return window.get();
    }
    return nullptr;
  }

 private:
  void onHideWindow(T *window) {
    for (auto iter = _list.begin(); iter != _list.end(); ++iter) {
      if (iter->get() == window) {
        _list.erase(iter);
        break;
      }
    }
  }

  std::vector<std::unique_ptr<T>> _list;
};

}  // namespace glight::gui

#endif
