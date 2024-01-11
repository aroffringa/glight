#ifndef GLIGHT_GUI_INSTANCE_H_
#define GLIGHT_GUI_INSTANCE_H_

namespace glight {
namespace theatre {
class Management;
}

namespace gui {

class EventTransmitter;

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

  theatre::Management& Management() { return *management_; }
  void SetManagement(theatre::Management& management) {
    management_ = &management;
  }

  EventTransmitter& Events() { return *events_; }
  void SetEvents(EventTransmitter& events) { events_ = &events; }

 private:
  theatre::Management* management_;
  EventTransmitter* events_;
};

}  // namespace gui
}  // namespace glight

#endif
