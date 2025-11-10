#ifndef GUI_FADERS_TOGGLE_WIDGET_H_
#define GUI_FADERS_TOGGLE_WIDGET_H_

#include "controlwidget.h"

#include "../components/controlbutton.h"
#include "../components/iconbutton.h"

#include <sigc++/scoped_connection.h>

#include <gtkmm/button.h>
#include <gtkmm/label.h>

namespace glight::gui {

class ToggleWidget final : public ControlWidget {
 public:
  ToggleWidget(FaderWindow &fader_window, FaderState &state, ControlMode mode,
               char key);

  virtual void Toggle() override;
  virtual void FlashOn() override;
  virtual void FlashOff() override;
  virtual void SyncFader() override;

  virtual void Limit(double value) override;

 private:
  void PrepareContextMenu(ControlMenu& menu) final {}

  virtual void OnAssigned(bool moveFader) override;
  void OnIconClicked();
  void OnFlashButtonPressed(int button);
  void OnFlashButtonReleased(int button);
  void OnFade();
  void UpdateDisplaySettings();
  void UpdateActivated(const theatre::SingleSourceValue &value);

  ControlButton flash_button_;
  Gtk::Button fade_button_;
  IconButton icon_button_;
  Gtk::Label name_label_{"<..>"};

  bool hold_updates_ = false;

  sigc::scoped_connection update_display_settings_connection_;
  size_t counter_ = 0;
};

}  // namespace glight::gui

#endif
