#ifndef GLIGHT_GUI_STATE_H_
#define GLIGHT_GUI_STATE_H_

#include <memory>
#include <vector>

#include <sigc++/signal.h>

#include "fadersetstate.h"

namespace glight::gui {

class GUIState {
 public:
  std::vector<std::unique_ptr<FaderSetState>> &FaderSets() {
    return fader_sets_;
  }
  const std::vector<std::unique_ptr<FaderSetState>> &FaderSets() const {
    return fader_sets_;
  }

  /**
   * Emitted when fadersetupstates are added or removed from the list.
   */
  sigc::signal<void()> &FaderSetSignalChange() {
    return fader_set_signal_change_;
  }

  void EmitFaderSetChangeSignal() { fader_set_signal_change_(); }

  void Clear() { fader_sets_.clear(); }

  bool Empty() const { return fader_sets_.empty(); }

  bool IsAssigned(const theatre::SourceValue *s) const {
    for (const std::unique_ptr<FaderSetState> &fader : fader_sets_) {
      if (fader->IsAssigned(s)) return true;
    }
    return false;
  }

 private:
  sigc::signal<void()> fader_set_signal_change_;
  std::vector<std::unique_ptr<FaderSetState>> fader_sets_;
};

}  // namespace glight::gui

#endif
