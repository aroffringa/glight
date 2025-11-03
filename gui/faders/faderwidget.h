#ifndef GUI_FADER_WIDGET_H_
#define GUI_FADER_WIDGET_H_

#include "controlwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/overlay.h>
#include <gtkmm/scale.h>

#include "../components/iconbutton.h"

#include "../../theatre/forwards.h"

namespace glight::gui {

class ControlMenu;

/**
 * @author Andre Offringa
 */
class FaderWidget final : public ControlWidget {
 public:
  FaderWidget(FaderWindow &fader_window, FaderState &state, ControlMode mode,
              char key);
  ~FaderWidget();

  void Toggle() override;
  void FlashOn() override;
  void FlashOff() override;
  void OnAssigned(bool moveFader) override;
  void SyncFader() override;

  void Limit(double value) override {
    if (_scale.get_value() > value) _scale.set_value(value);
  }

  Gtk::Widget &NameLabel() { return _nameLabel; }

 private:
  void ShowFadeButtons(bool mouse_in);
  void onScaleChange();
  void onOnButtonClicked();
  void onFlashButtonPressed(int, double, double);
  void onFlashButtonReleased(int, double, double);
  void onFadeUp();
  void onFadeDown();
  void HandleRightPress(int, double, double);
  void HandleRightRelease(int, double, double);
  void MakeMenu();
  void UpdateDisplaySettings();

  Gtk::Overlay _overlay;
  Gtk::Button _fadeUpButton;
  Gtk::Scale _scale;
  Gtk::Button _fadeDownButton;
  Gtk::Button _flashButton;
  Gtk::Label flash_button_label_;
  IconButton _checkButton;
  Gtk::Label _nameLabel{"<..>"};
  bool _mouseIn = false;
  bool _holdUpdates = false;
  sigc::connection update_display_settings_connection_;
};

}  // namespace glight::gui

#endif
