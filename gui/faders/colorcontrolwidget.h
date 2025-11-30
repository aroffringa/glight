#ifndef GUI_FADERS_COLOR_CONTROL_WIDGET_H_
#define GUI_FADERS_COLOR_CONTROL_WIDGET_H_

#include "controlwidget.h"

#include "gui/components/colorselectwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>

namespace glight::gui {

class ColorControlWidget final : public ControlWidget {
 public:
  ColorControlWidget(FaderWindow &fader_window, FaderState &state,
                     ControlMode mode, char key);
  ~ColorControlWidget();

  void Toggle() final;
  void FlashOn() final;
  void FlashOff() final;
  void SyncFader() final;

  void Limit(double value) final;

  void SetColor(theatre::Color color) {
    previous_color_ = color_selector_.GetColor();
    color_selector_.SetColor(color);
  }

 private:
  void OnAssigned(bool moveFader) final;
  void PrepareContextMenu(ControlMenu &menu) final;
  void UpdateDisplaySettings();
  void OnColorChanged();
  theatre::Color ColorFromSourceValues() const;
  void ShowAssignControllableDialog();

  Gtk::Label name_label_{"<..>"};
  ColorSelectWidget color_selector_;

  bool hold_updates_ = false;

  sigc::connection update_display_settings_connection_;
  theatre::Color previous_color_ = theatre::Color::Black();
  std::unique_ptr<Gtk::Dialog> dialog_;
};

}  // namespace glight::gui

#endif
