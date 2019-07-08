#ifndef EVENT_TRANSMITTER_H
#define EVENT_TRANSMITTER_H

class EventTransmitter
{
public:
	virtual void EmitUpdate() = 0;
};

#endif
