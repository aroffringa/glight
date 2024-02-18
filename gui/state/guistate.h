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
   * Emitted when fader sets are added or removed from the list. This
   * is not emitted when individual faders are changed; it is only for
   * situations in which the list of fader sets are shown, e.g. as for
   * the menu items in the main window.
   */
  sigc::signal<void()> &FaderSetSignalChange() {
    return fader_set_signal_change_;
  }

  void EmitFaderSetChangeSignal() { fader_set_signal_change_(); }

  void Clear() {
    fader_sets_.clear();
    layout_locked_ = false;
  }

  bool Empty() const { return fader_sets_.empty(); }

  bool IsAssigned(const theatre::SourceValue *s) const {
    for (const std::unique_ptr<FaderSetState> &fader : fader_sets_) {
      if (fader->IsAssigned(s)) return true;
    }
    return false;
  }

  bool LayoutLocked() const { return layout_locked_; }
  void SetLayoutLocked(bool layout_locked) { layout_locked_ = layout_locked; }

  /**
   * Returns the first unassigned controller. It only considers the types fader
   * or toggle buttons.
   */
  FaderState *GetFirstUnassignedFader() {
    for (std::unique_ptr<FaderSetState> &set : fader_sets_) {
      FaderState *fader = set->GetFirstUnassigned();
      if (fader) return fader;
    }
    return nullptr;
  }

 private:
  bool layout_locked_ = false;
  sigc::signal<void()> fader_set_signal_change_;
  std::vector<std::unique_ptr<FaderSetState>> fader_sets_;
};

}  // namespace glight::gui

#endif
