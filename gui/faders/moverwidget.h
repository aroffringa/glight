#ifndef GLIGHT_GUI_FADERS_MOVER_WIDGET_H_
#define GLIGHT_GUI_FADERS_MOVER_WIDGET_H_

#include "controlwidget.h"

#include "gui/components/controlbutton.h"

#include <sigc++/scoped_connection.h>

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>

namespace glight::gui {

class MoverWidget final : public ControlWidget {
 public:
  MoverWidget(FaderWindow &fader_window, FaderState &state, ControlMode mode,
              char key);

  void Toggle() final {}
  void FlashOn() final {}
  void FlashOff() final {}
  void SyncFader() final {}

  void Limit(double value) final {}

  bool PanIsAssigned() const { return GetSourceValue(0) != nullptr; }
  bool TiltIsAssigned() const { return GetSourceValue(1) != nullptr; }

 private:
  void OnAssigned(bool moveFader) final;
  void HandleRightRelease();
  void UpdateDisplaySettings();
  void MoveLeft();
  void MoveRight();
  void StopPan();
  void MoveUp();
  void MoveDown();
  void StopTilt();

  Gtk::Grid grid_;
  ControlButton left_button_;
  ControlButton right_button_;
  ControlButton up_button_;
  ControlButton down_button_;
  Gtk::Label name_label_{"<..>\n<..>"};

  bool hold_updates_ = false;

  sigc::scoped_connection update_display_settings_connection_;
};

}  // namespace glight::gui

#endif
