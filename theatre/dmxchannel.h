#ifndef DMXCHANNEL_H
#define DMXCHANNEL_H

#include "controlvalue.h"

/**
 * @author Andre Offringa
 */
class DmxChannel {
 public:
  DmxChannel()
      : _universe(0), _channel(0), _defaultMixStyle(ControlValue::Default) {}
  DmxChannel(unsigned channel, unsigned universe)
      : _universe(universe),
        _channel(channel),
        _defaultMixStyle(ControlValue::Default) {}
  ~DmxChannel() {}

  unsigned Universe() const { return _universe; }
  unsigned Channel() const { return _channel; }
  ControlValue::MixStyle DefaultMixStyle() const { return _defaultMixStyle; }

  void SetUniverse(unsigned universe) { _universe = universe; }
  void SetChannel(unsigned channel) { _channel = channel; }
  void SetDefaultMixStyle(ControlValue::MixStyle defaultMixStyle) {
    _defaultMixStyle = defaultMixStyle;
  }

 private:
  unsigned _universe;
  unsigned _channel;
  ControlValue::MixStyle _defaultMixStyle;
};

#endif
