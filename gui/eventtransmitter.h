#ifndef GUI_EVENT_TRANSMITTER_H_
#define GUI_EVENT_TRANSMITTER_H_

#include <sigc++/signal.h>

namespace glight::gui {

class EventTransmitter {
 public:
  virtual ~EventTransmitter() = default;
  virtual void EmitUpdate() = 0;

  virtual sigc::signal<void()> &SignalUpdateControllables() = 0;
};

}  // namespace glight::gui

#endif
