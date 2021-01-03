#ifndef FIXTUREFUNCTION_H
#define FIXTUREFUNCTION_H

#include <string>
#include <vector>

#include "controlvalue.h"
#include "dmxchannel.h"
#include "functiontype.h"
#include "namedobject.h"
#include "valuesnapshot.h"

/**
 * @author Andre Offringa
 */
class FixtureFunction : public NamedObject {
 public:
  FixtureFunction(class Theatre &theatre, FunctionType type,
                  const std::string &name);

  FixtureFunction(class Theatre &theatre, FunctionType type);

  FixtureFunction(const FixtureFunction &source, class Theatre &theatre);

  FixtureFunction(const FixtureFunction &source) = delete;
  FixtureFunction &operator=(const FixtureFunction &source) = delete;

  void MixChannels(unsigned value, MixStyle mixStyle, unsigned *channels,
                   unsigned universe) {
    if (_firstChannel.Universe() != universe)
      throw std::runtime_error("Can't handle multiple universes");
    MixStyle combiMixStyle = ControlValue::CombineMixStyles(
        mixStyle, _firstChannel.DefaultMixStyle());
    if (IsSingleChannel()) {
      channels[_firstChannel.Channel()] = ControlValue::Mix(
          channels[_firstChannel.Channel()], value, combiMixStyle);
    } else {  // 16 bit
      const unsigned currentValue =
          (channels[_firstChannel.Channel()]) +
          (channels[_firstChannel.Channel() + 1] >> 8);
      const unsigned mixedValue =
          ControlValue::Mix(currentValue, value, combiMixStyle);
      // Set to the first 8 of 24 bits.
      channels[_firstChannel.Channel()] = (mixedValue & (~0xFFFF));
      // Set to bits 9-24.
      channels[_firstChannel.Channel() + 1] = (mixedValue & 0xFFFF) << 8;
    }
  }

  void SetChannel(const DmxChannel &channel, bool is16Bit = false);
  bool IsSingleChannel() const { return !_is16Bit; }
  const DmxChannel &FirstChannel() const { return _firstChannel; }
  void IncChannel();
  void DecChannel();
  enum FunctionType Type() const { return _type; }
  unsigned char GetValue(const ValueSnapshot &snapshot) const {
    return snapshot.GetValue(_firstChannel);
  }

 private:
  class Theatre &_theatre;
  enum FunctionType _type;
  DmxChannel _firstChannel;
  bool _is16Bit;
};

#endif
