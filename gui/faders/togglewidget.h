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

class ToggleWidget : public ControlWidget {
 public:
  ToggleWidget(theatre::Management &management, EventTransmitter &eventHub,
               char key);
  ~ToggleWidget();

  virtual void Toggle() final override;
  virtual void FullOn() final override;
  virtual void FullOff() final override;
  virtual void Assign(theatre::SourceValue *item,
                      bool moveFader) final override;
  virtual void MoveSlider() final override;
  virtual theatre::SourceValue *GetSourceValue() const final override {
    return _sourceValue;
  }

  virtual void Limit(double value) final override;
  virtual void ChangeManagement(theatre::Management &management,
                                bool moveSliders) final override;

 private:
  Gtk::HBox _box;
  Gtk::VBox _flashButtonBox;
  Gtk::Button _flashButton;
  Gtk::CheckButton _onCheckButton;
  Gtk::EventBox _eventBox;
  Gtk::Label _nameLabel;

  sigc::connection _updateConnection;
  theatre::Management *_management;
  EventTransmitter &_eventHub;
  theatre::SourceValue *_sourceValue;

  bool _holdUpdates;

  void onUpdate();
  void onOnButtonClicked();
  bool onNameLabelClicked(GdkEventButton *event);
  bool onFlashButtonPressed(GdkEventButton *event);
  bool onFlashButtonReleased(GdkEventButton *event);
};

}  // namespace glight::gui

#endif
