#ifndef GLIGHT_GUI_FADERS_MOVER_WIDGET_H_
#define GLIGHT_GUI_FADERS_MOVER_WIDGET_H_

#include "controlwidget.h"

#include "../scopedconnection.h"

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>

namespace glight::gui {

class MoverWidget final : public ControlWidget {
 public:
  MoverWidget(FaderWindow &fader_window, FaderState &state, ControlMode mode,
              char key);

  virtual void Toggle() override {}
  virtual void FlashOn() override {}
  virtual void FlashOff() override {}
  virtual void SyncFader() override {}

  virtual void Limit(double value) override {}

 private:
  virtual void OnAssigned(bool moveFader) override;
  bool HandleRightRelease(GdkEventButton *event);
  void UpdateDisplaySettings();
  void MoveLeft();
  void MoveRight();
  void StopPan();
  void MoveUp();
  void MoveDown();
  void StopTilt();

  Gtk::Grid grid_;
  Gtk::Button left_button_;
  Gtk::Button right_button_;
  Gtk::Button up_button_;
  Gtk::Button down_button_;
  Gtk::EventBox event_box_;
  Gtk::Label name_label_{"<..>\n<..>"};

  bool hold_updates_ = false;

  ScopedConnection update_display_settings_connection_;
};

}  // namespace glight::gui

#endif
