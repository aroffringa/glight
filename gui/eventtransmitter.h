#ifndef EVENT_TRANSMITTER_H
#define EVENT_TRANSMITTER_H

#include <sigc++/signal.h>

class EventTransmitter {
public:
  virtual void EmitUpdate() = 0;

  virtual sigc::signal<void(class Management &)> &SignalChangeManagement() = 0;

  virtual sigc::signal<void()> &SignalUpdateControllables() = 0;
};

#endif
