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

class EventTransmitter;

/**
 * @author Andre Offringa
 */
class FaderWidget : public ControlWidget {
 public:
  FaderWidget(theatre::Management &management, EventTransmitter &eventHub,
              char key);
  ~FaderWidget();

  void Toggle() final override;
  void FullOn() final override;
  void FullOff() final override;
  void Assign(theatre::SourceValue *item, bool moveFader) final override;
  void MoveSlider() final override;
  theatre::SourceValue *GetSourceValue() const final override {
    return _sourceValue;
  }

  void Limit(double value) final override {
    if (_scale.get_value() > value) _scale.set_value(value);
  }

  void ChangeManagement(theatre::Management &management,
                        bool moveSliders) final override;

  Gtk::Widget &NameLabel() { return _eventBox; }

 private:
  void onUpdate();
  void onScaleChange();
  void onOnButtonClicked();
  bool onNameLabelClicked(GdkEventButton *event);
  bool onFlashButtonPressed(GdkEventButton *event);
  bool onFlashButtonReleased(GdkEventButton *event);

  Gtk::VBox _box;
  Gtk::VScale _scale;
  Gtk::Button _flashButton;
  Gtk::CheckButton _onCheckButton;
  Gtk::EventBox _eventBox;
  Gtk::Label _nameLabel;

  sigc::connection _updateConnection;
  theatre::Management *_management;
  EventTransmitter &_eventHub;
  theatre::SourceValue *_sourceValue;

  bool _holdUpdates;
};

}  // namespace glight::gui

#endif
