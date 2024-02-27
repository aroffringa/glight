#ifndef GUI_TIME_SEQUENCE_PROPERTIES_WINDOW_H_
#define GUI_TIME_SEQUENCE_PROPERTIES_WINDOW_H_

#include "propertieswindow.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/separator.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "gui/recursionlock.h"
#include "gui/scopedconnection.h"

#include "gui/components/durationinput.h"
#include "gui/components/inputselectwidget.h"
#include "gui/components/transitiontypebox.h"

#include "theatre/forwards.h"
#include "theatre/timesequence.h"
#include "theatre/transition.h"

namespace glight::gui {

class EventTransmitter;

/**
 * @author Andre Offringa
 */
class TimeSequencePropertiesWindow final : public PropertiesWindow {
 public:
  TimeSequencePropertiesWindow(theatre::TimeSequence &timeSequence);

  theatre::FolderObject &GetObject() override;
  theatre::TimeSequence &GetTimeSequence() { return *_timeSequence; }

 private:
  void onInputSelectionChanged();
  void onSelectedStepChanged();
  void load();
  void fillStepsList();
  void loadStep(const theatre::TimeSequence::Step &step);
  void onAddStep();
  void onRemoveStep();
  void onSustainChanged();
  void onRepeatChanged();
  void onTriggerTypeChanged();
  void onTriggerSpeedChanged(double newValue);
  void onTransitionSpeedChanged(double newValue);
  void onTransitionTypeChanged(theatre::TransitionType type);
  void onSyncCountChanged();
  void onBeatSpeedChanged();

  void onUpdateControllables();
  void setStepSensitive(bool sensitive);
  theatre::TimeSequence::Step *selectedStep();
  void selectStep(size_t index);

  Gtk::HBox _topBox;
  InputSelectWidget _inputSelector;

  Gtk::VBox _buttonBox;
  Gtk::Button _addStepButton;
  Gtk::Button _removeStepButton;

  Gtk::Grid _grid;
  Gtk::TreeView _stepsView;
  Glib::RefPtr<Gtk::ListStore> _stepsStore;
  struct StepsListColumns : public Gtk::TreeModelColumnRecord {
    StepsListColumns() {
      add(_title);
      add(_trigger);
      add(_step);
    }

    Gtk::TreeModelColumn<Glib::ustring> _title;
    Gtk::TreeModelColumn<Glib::ustring> _trigger;
    Gtk::TreeModelColumn<size_t> _step;
  } _stepsListColumns;
  Gtk::ScrolledWindow _stepsScrolledWindow;

  Gtk::CheckButton _sustainCB;
  Gtk::CheckButton _maxRepeatCB;
  Gtk::Scale _maxRepeatCount;

  Gtk::RadioButton _delayTriggerCheckButton;
  DurationInput _triggerDuration;

  Gtk::RadioButton _synchronizedTriggerCheckButton;
  Gtk::Scale _synchronizationsCount;

  Gtk::RadioButton _beatTriggerCheckButton;
  Gtk::Scale _beatSpeed;

  Gtk::Label _transitionSpeedLabel;
  DurationInput _transitionDuration;
  TransitionTypeBox _transitionTypeBox;

  RecursionLock _recursionLock;

  theatre::TimeSequence *_timeSequence;
  ScopedConnection update_connection_;
};

}  // namespace glight::gui

#endif
