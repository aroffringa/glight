#include "controlmenu.h"

#include "../state/faderstate.h"

namespace glight::gui {

ControlMenu::ControlMenu(const FaderState& state) {
  append(_miAssign);
  append(_miSeperator1);
  _miDisplayName.set_active(state.DisplayName());
  append(_miDisplayName);
  _miDisplayFlashButton.set_active(state.DisplayFlashButton());
  append(_miDisplayFlashButton);
  _miDisplayCheckButton.set_active(state.DisplayCheckButton());
  append(_miDisplayCheckButton);
  _miOverlayFadeButtons.set_active(state.OverlayFadeButtons());
  append(_miOverlayFadeButtons);

  show_all_children();
}

}  // namespace glight::gui
