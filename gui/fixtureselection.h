#ifndef GUI_FIXTURE_SELECTION_H_
#define GUI_FIXTURE_SELECTION_H_

#include <sigc++/signal.h>

#include <vector>

#include "../theatre/forwards.h"

namespace glight::gui {

class FixtureSelection {
 public:
  sigc::signal<void> &SignalChange() { return _signalChange; }

  const std::vector<theatre::Fixture *> &Selection() const {
    return _selection;
  }

  void SetSelection(const std::vector<theatre::Fixture *> &selection) {
    _selection = selection;
    _signalChange.emit();
  }
  void SetSelection(std::vector<theatre::Fixture *> &&selection) {
    _selection = std::move(selection);
    _signalChange.emit();
  }

 private:
  sigc::signal<void> _signalChange;
  std::vector<theatre::Fixture *> _selection;
};

}  // namespace glight::gui

#endif
