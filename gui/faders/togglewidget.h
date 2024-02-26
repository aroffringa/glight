#ifndef GUI_FADERS_TOGGLE_WIDGET_H_
#define GUI_FADERS_TOGGLE_WIDGET_H_

#include "controlwidget.h"

#include "../components/iconbutton.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>

namespace glight::gui {

class ToggleWidget final : public ControlWidget {
 public:
  ToggleWidget(FaderWindow &fader_window, FaderState &state, ControlMode mode,
               char key);
  ~ToggleWidget();

  virtual void Toggle() override;
  virtual void FlashOn() override;
  virtual void FlashOff() override;
  virtual void SyncFader() override;

  virtual void Limit(double value) override;

 private:
  Gtk::HBox box_;
  Gtk::Label flash_button_label_;
  Gtk::Button flash_button_;
  Gtk::Button fade_button_;
  IconButton icon_button_;
  Gtk::EventBox event_box_;
  Gtk::Label name_label_{"<..>"};

  bool hold_updates_ = false;

  sigc::connection update_display_settings_connection_;
  size_t counter_ = 0;

  virtual void OnAssigned(bool moveFader) override;
  void OnIconClicked();
  bool OnFlashButtonPressed(GdkEventButton *event);
  bool OnFlashButtonReleased(GdkEventButton *event);
  void OnFade();
  bool HandleRightRelease(GdkEventButton *event);
  void UpdateDisplaySettings();
  void UpdateActivated(const theatre::SingleSourceValue &value);
};

}  // namespace glight::gui

#endif
