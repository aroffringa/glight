#ifndef GLIGHT_GUI_WINDOWS_CHILD_WINDOW_LIST_H_
#define GLIGHT_GUI_WINDOWS_CHILD_WINDOW_LIST_H_

#include <algorithm>
#include <functional>
#include <vector>

#include "childwindow.h"

namespace glight::gui::windows {

class ChildWindowList {
 public:
  template <typename WindowType, typename... WindowArguments>
  WindowType& Open(
      std::function<void()> on_close = []() {},
      WindowArguments&&... arguments) {
    auto iter = Get<WindowType>();
    if (iter == children_.end()) {
      WindowData& data = children_.emplace_back(
          WindowData{std::make_unique<WindowType>(
                         std::forward<WindowArguments>(arguments)...),
                     std::move(on_close)});
      ChildWindow* pointer = data.window.get();
      data.window->signal_close_request().connect(
          [this, pointer]() -> bool {
            auto iter = std::find_if(children_.begin(), children_.end(),
                                     [pointer](const WindowData& item) {
                                       return item.window.get() == pointer;
                                     });
            iter->on_hide();
            children_.erase(iter);
            return false;
          },
          false);
      data.window->show();
      return static_cast<WindowType&>(*data.window);
    } else {
      iter->window->present();
      return static_cast<WindowType&>(*iter->window);
    }
  }

  template <typename WindowType>
  void Hide() {
    auto iter = Get<WindowType>();
    if (iter != children_.end()) {
      iter->window->hide();
    }
  }

  void SetLayoutLocked(bool layout_is_locked) {
    for (WindowData& child : children_) {
      child.window->SetLayoutLocked(layout_is_locked);
    }
  }

  void Clear() { children_.clear(); }

 private:
  struct WindowData {
    std::unique_ptr<ChildWindow> window;
    std::function<void()> on_hide;
  };

  template <typename WindowType>
  std::vector<WindowData>::iterator Get() {
    return std::find_if(
        children_.begin(), children_.end(), [](const WindowData& data) {
          return dynamic_cast<const WindowType*>(data.window.get()) != nullptr;
        });
  }

  std::vector<WindowData> children_;
};

}  // namespace glight::gui::windows

#endif
