#ifndef GUI_CHASE_PROPERTIES_WINDOW_H_
#define GUI_CHASE_PROPERTIES_WINDOW_H_

#include "propertieswindow.h"

#include "../../theatre/forwards.h"

#include "../components/durationinput.h"
#include "../components/transitiontypebox.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/separator.h>
#include <gtkmm/window.h>

namespace glight::gui {

class EventTransmitter;

class ChasePropertiesWindow : public PropertiesWindow {
 public:
  ChasePropertiesWindow(theatre::Chase &chase, theatre::Management &management,
                        EventTransmitter &eventHub);
  ~ChasePropertiesWindow();

  theatre::FolderObject &GetObject() final override;
  theatre::Chase &GetChase() { return *_chase; }

 private:
  void loadChaseInfo(theatre::Chase &chase);
  void onTriggerTypeChanged();
  void onTriggerSpeedChanged(double newValue);
  void onTransitionSpeedChanged(double newValue);
  void onTransitionTypeChanged(theatre::TransitionType type);
  void onSyncCountChanged();
  void onBeatSpeedChanged();

  void onChangeManagement(theatre::Management &management) {
    _management = &management;
  }
  void onUpdateControllables();
  void onToTimeSequenceClicked();

  Gtk::VBox _box;
  Gtk::Grid _grid;

  Gtk::RadioButton _delayTriggerCheckButton;
  DurationInput _triggerDuration;
  DurationInput _transitionDuration;
  TransitionTypeBox _transitionTypeBox;
  Gtk::HSeparator _transitionSep;

  Gtk::RadioButton _synchronizedTriggerCheckButton;
  Gtk::Label _synchronizationsLabel;
  Gtk::HScale _synchronizationsCount;
  Gtk::HSeparator _synchronizedSep;

  Gtk::RadioButton _beatTriggerCheckButton;
  Gtk::Label _beatSpeedLabel;
  Gtk::HScale _beatSpeed;

  Gtk::HButtonBox _buttonBox;
  Gtk::Button _toTimeSequenceButton, _closeButton;

  theatre::Chase *_chase;
  theatre::Management *_management;
  EventTransmitter &_eventHub;
};

}  // namespace glight::gui

#endif
