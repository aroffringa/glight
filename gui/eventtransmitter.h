#ifndef GUI_EVENT_TRANSMITTER_H_
#define GUI_EVENT_TRANSMITTER_H_

#include <sigc++/signal.h>

#include "../theatre/forwards.h"

namespace glight::gui {

class EventTransmitter {
 public:
  virtual void EmitUpdate() = 0;

  virtual sigc::signal<void(theatre::Management &)>
      &SignalChangeManagement() = 0;

  virtual sigc::signal<void()> &SignalUpdateControllables() = 0;
};

}  // namespace glight::gui

#endif
