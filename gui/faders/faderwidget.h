#ifndef GUI_FADER_WIDGET_H_
#define GUI_FADER_WIDGET_H_

#include "controlwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/eventbox.h>
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

  Gtk::Widget &NameLabel() { return _labelEventBox; }

 private:
  void ShowFadeButtons(bool mouse_in);
  void onScaleChange();
  void onOnButtonClicked();
  bool onFlashButtonPressed(GdkEventButton *event);
  bool onFlashButtonReleased(GdkEventButton *event);
  void onFadeUp();
  void onFadeDown();
  bool HandleRightPress(GdkEventButton *event);
  bool HandleRightRelease(GdkEventButton *event);
  void MakeMenu();
  void UpdateDisplaySettings();

  Gtk::EventBox _mouseInBox;
  Gtk::Overlay _overlay;
  Gtk::VBox _box;
  Gtk::Button _fadeUpButton;
  Gtk::Scale _scale;
  Gtk::Button _fadeDownButton;
  Gtk::Button _flashButton;
  Gtk::Label flash_button_label_;
  IconButton _checkButton;
  Gtk::EventBox _labelEventBox;
  Gtk::Label _nameLabel{"<..>"};
  bool _mouseIn = false;
  bool _holdUpdates = false;
  sigc::connection update_display_settings_connection_;
};

}  // namespace glight::gui

#endif
