#ifndef GLIGHT_GUI_INSTANCE_H_
#define GLIGHT_GUI_INSTANCE_H_

#include <memory>

namespace Gtk {
class ApplicationWindow;
}

namespace glight {
namespace theatre {
class Management;
}

namespace system {
struct Settings;
}

namespace gui {

class EventTransmitter;
class FixtureSelection;
class GUIState;

/**
 * Class to store data that is unique to the instance and that
 * is accessed in many places.
 */
class Instance {
 public:
  Instance();
  ~Instance();

  Instance(const Instance&) = delete;
  Instance& operator=(const Instance&) = delete;

  static Instance& Get() {
    static Instance instance;
    return instance;
  }

  static theatre::Management& Management() { return *Get().management_; }
  void SetManagement(theatre::Management& management) {
    management_ = &management;
  }

  static EventTransmitter& Events() { return *Get().events_; }
  void SetEvents(EventTransmitter& events) { events_ = &events; }

  static FixtureSelection& Selection() { return *Get().selection_; }
  void SetSelection(FixtureSelection& selection) { selection_ = &selection; }

  static GUIState& State() { return *Get().state_; }
  void SetState(GUIState& state) { state_ = &state; }

  static system::Settings& Settings() { return *Get().settings_; }

  static Gtk::ApplicationWindow& MainWindow() { return *Get().main_window_; }
  void SetMainWindow(Gtk::ApplicationWindow& main_window) {
    main_window_ = &main_window;
  }

 private:
  // Because this class is used in many places, all members are pointers
  // so that extra includes can be avoided.
  theatre::Management* management_;
  EventTransmitter* events_;
  FixtureSelection* selection_;
  GUIState* state_;
  Gtk::ApplicationWindow* main_window_;
  std::unique_ptr<system::Settings> settings_;
};

}  // namespace gui
}  // namespace glight

#endif
