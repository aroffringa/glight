#ifndef THEATRE_DMXCHANNEL_H_
#define THEATRE_DMXCHANNEL_H_

#include "controlvalue.h"
#include "mixstyle.h"

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class DmxChannel {
 public:
  DmxChannel()
      : _universe(0), _channel(0), _defaultMixStyle(MixStyle::Default) {}
  DmxChannel(unsigned channel, unsigned universe)
      : _universe(universe),
        _channel(channel),
        _defaultMixStyle(MixStyle::Default) {}
  ~DmxChannel() {}

  unsigned Universe() const { return _universe; }
  unsigned Channel() const { return _channel; }
  MixStyle DefaultMixStyle() const { return _defaultMixStyle; }

  void SetUniverse(unsigned universe) { _universe = universe; }
  void SetChannel(unsigned channel) { _channel = channel; }
  void SetDefaultMixStyle(MixStyle defaultMixStyle) {
    _defaultMixStyle = defaultMixStyle;
  }
  DmxChannel Next() const {
    return DmxChannel((_channel + 1) % 512, _universe);
  }

 private:
  unsigned _universe;
  unsigned _channel;
  MixStyle _defaultMixStyle;
};

}  // namespace glight::theatre

#endif
