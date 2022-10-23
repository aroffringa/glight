#ifndef GUI_FADER_WIDGET_H_
#define GUI_FADER_WIDGET_H_

#include "controlwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../../theatre/forwards.h"

namespace glight::gui {

/**
 * @author Andre Offringa
 */
class FaderWidget final : public ControlWidget {
 public:
  FaderWidget(theatre::Management &management, EventTransmitter &eventHub,
              ControlMode mode, char key);

  void Toggle() override;
  void FullOn() override;
  void FullOff() override;
  void OnAssigned(bool moveFader) override;
  void MoveSlider() override;

  void Limit(double value) override {
    if (_scale.get_value() > value) _scale.set_value(value);
  }

  Gtk::Widget &NameLabel() { return _eventBox; }

 private:
  void onScaleChange();
  void onOnButtonClicked();
  bool onNameLabelClicked(GdkEventButton *event);
  bool onFlashButtonPressed(GdkEventButton *event);
  bool onFlashButtonReleased(GdkEventButton *event);
  void onFadeUp();
  void onFadeDown();

  Gtk::VBox _box;
  Gtk::Button _fadeUpButton;
  Gtk::VScale _scale;
  Gtk::Button _fadeDownButton;
  Gtk::Button _flashButton;
  Gtk::CheckButton _onCheckButton;
  Gtk::EventBox _eventBox;
  Gtk::Label _nameLabel;

  bool _holdUpdates;
};

}  // namespace glight::gui

#endif
