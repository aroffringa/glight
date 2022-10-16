#ifndef GUI_FADERS_TOGGLE_WIDGET_H_
#define GUI_FADERS_TOGGLE_WIDGET_H_

#include "controlwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>

namespace glight::gui {

class EventTransmitter;

class ToggleWidget final : public ControlWidget {
 public:
  ToggleWidget(theatre::Management &management, EventTransmitter &eventHub,
               char key);
  ~ToggleWidget();

  virtual void Toggle() override;
  virtual void FullOn() override;
  virtual void FullOff() override;
  virtual void MoveSlider() override;

  virtual void Limit(double value) override;

 private:
  Gtk::HBox _box;
  Gtk::VBox _flashButtonBox;
  Gtk::Button _flashButton;
  Gtk::CheckButton _onCheckButton;
  Gtk::EventBox _eventBox;
  Gtk::Label _nameLabel;

  sigc::connection _updateConnection;

  bool _holdUpdates;

  virtual void OnAssigned(bool moveFader) override;
  void onUpdate();
  void onOnButtonClicked();
  bool onNameLabelClicked(GdkEventButton *event);
  bool onFlashButtonPressed(GdkEventButton *event);
  bool onFlashButtonReleased(GdkEventButton *event);
};

}  // namespace glight::gui

#endif
