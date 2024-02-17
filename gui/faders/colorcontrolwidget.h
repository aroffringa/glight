#ifndef GUI_FADERS_COLOR_CONTROL_WIDGET_H_
#define GUI_FADERS_COLOR_CONTROL_WIDGET_H_

#include "controlwidget.h"

#include "gui/components/colorselectwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>

namespace glight::gui {

class ColorControlWidget final : public ControlWidget {
 public:
  ColorControlWidget(FaderWindow &fader_window, FaderState &state,
                     ControlMode mode, char key);
  ~ColorControlWidget();

  virtual void Toggle() override;
  virtual void FlashOn() override;
  virtual void FlashOff() override;
  virtual void SyncFader() override;

  virtual void Limit(double value) override;

  void SetColor(theatre::Color color) {
    previous_color_ = color_selector_.GetColor();
    color_selector_.SetColor(color);
  }

 private:
  virtual void OnAssigned(bool moveFader) override;
  void UpdateDisplaySettings();
  void OnColorChanged();
  theatre::Color ColorFromSourceValues() const;
  bool HandleRightRelease(GdkEventButton *event);
  void ShowAssignControllableDialog();

  Gtk::HBox box_;
  Gtk::EventBox event_box_;
  Gtk::Label name_label_{"<..>"};
  ColorSelectWidget color_selector_;

  bool hold_updates_ = false;

  sigc::connection update_display_settings_connection_;
  theatre::Color previous_color_ = theatre::Color::Black();
};

}  // namespace glight::gui

#endif
