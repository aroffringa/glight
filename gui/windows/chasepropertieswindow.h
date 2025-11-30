#ifndef GUI_CHASE_PROPERTIES_WINDOW_H_
#define GUI_CHASE_PROPERTIES_WINDOW_H_

#include "propertieswindow.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/separator.h>
#include <gtkmm/window.h>

#include "theatre/forwards.h"

#include <sigc++/scoped_connection.h>
#include "gui/components/beatinput.h"
#include "gui/components/durationinput.h"
#include "gui/components/transitiontypebox.h"

namespace glight::gui {

class ChasePropertiesWindow : public PropertiesWindow {
 public:
  ChasePropertiesWindow(theatre::Chase &chase);
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
  void onBeatSpeedChanged(double value);

  void onUpdateControllables();
  void onToTimeSequenceClicked();

  Gtk::Box _box{Gtk::Orientation::VERTICAL};
  Gtk::Grid _grid;

  Gtk::CheckButton _delayTriggerCheckButton;
  DurationInput _triggerDuration;
  DurationInput _transitionDuration;
  TransitionTypeBox _transitionTypeBox;
  Gtk::Separator _transitionSep;

  Gtk::CheckButton _synchronizedTriggerCheckButton;
  Gtk::Label _synchronizationsLabel;
  Gtk::Scale _synchronizationsCount;
  Gtk::Separator _synchronizedSep;

  Gtk::CheckButton _beatTriggerCheckButton;
  Gtk::Label _beatSpeedLabel;
  BeatInput _beatSpeed;

  Gtk::Box _buttonBox;
  Gtk::Button _toTimeSequenceButton, _closeButton;
  sigc::scoped_connection update_connection_;

  theatre::Chase *_chase;
};

}  // namespace glight::gui

#endif
