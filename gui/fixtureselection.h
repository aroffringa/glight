#ifndef GUI_FIXTURE_SELECTION_H_
#define GUI_FIXTURE_SELECTION_H_

#include <sigc++/signal.h>

#include <vector>

#include "../theatre/forwards.h"
#include "system/trackableptr.h"

namespace glight::gui {

class FixtureSelection {
 public:
  sigc::signal<void> &SignalChange() { return _signalChange; }

  const std::vector<system::ObservingPtr<theatre::Fixture>> &Selection() const {
    return _selection;
  }

  void SetSelection(
      const std::vector<system::ObservingPtr<theatre::Fixture>> &selection) {
    _selection = selection;
    _signalChange.emit();
  }
  void SetSelection(
      std::vector<system::ObservingPtr<theatre::Fixture>> &&selection) {
    _selection = std::move(selection);
    _signalChange.emit();
  }
  /**
   * If any fixtures are in the selection that no longer exist, remove them.
   */
  void UpdateAfterDelete() {
    const auto iter =
        std::remove(_selection.begin(), _selection.end(), nullptr);
    if (iter != _selection.end()) {
      _selection.erase(iter, _selection.end());
      _signalChange.emit();
    }
  }

 private:
  sigc::signal<void> _signalChange;
  std::vector<system::ObservingPtr<theatre::Fixture>> _selection;
};

}  // namespace glight::gui

#endif
