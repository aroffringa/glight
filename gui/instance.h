#ifndef GLIGHT_GUI_INSTANCE_H_
#define GLIGHT_GUI_INSTANCE_H_

namespace glight {
namespace theatre {
class Management;
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

 private:
  theatre::Management* management_;
  EventTransmitter* events_;
  FixtureSelection* selection_;
  GUIState* state_;
};

}  // namespace gui
}  // namespace glight

#endif
