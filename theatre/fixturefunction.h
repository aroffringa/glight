#ifndef THEATRE_FIXTUREFUNCTION_H_
#define THEATRE_FIXTUREFUNCTION_H_

#include <string>
#include <vector>

#include "controlvalue.h"
#include "dmxchannel.h"
#include "functiontype.h"
#include "namedobject.h"
#include "valuesnapshot.h"

namespace glight::theatre {

class Theatre;

/**
 * @author Andre Offringa
 */
class FixtureFunction final : public NamedObject {
 public:
  FixtureFunction(Theatre &theatre, FunctionType type, const std::string &name);

  FixtureFunction(Theatre &theatre, FunctionType type);

  FixtureFunction(const FixtureFunction &source, Theatre &theatre);

  FixtureFunction(const FixtureFunction &source) = delete;
  FixtureFunction &operator=(const FixtureFunction &source) = delete;

  void MixChannels(unsigned value, MixStyle mixStyle, unsigned *channels,
                   unsigned universe) {
    if (_firstChannel.Universe() == universe) {
      MixStyle combiMixStyle = ControlValue::CombineMixStyles(
          mixStyle, _firstChannel.DefaultMixStyle());
      if (IsSingleChannel()) {
        channels[_firstChannel.Channel()] = ControlValue::Mix(
            channels[_firstChannel.Channel()], value, combiMixStyle);
      } else {  // 16 bit
        const unsigned currentValue =
            (channels[_firstChannel.Channel()]) +
            (channels[_firstChannel.Next().Channel()] >> 8);
        const unsigned mixedValue =
            ControlValue::Mix(currentValue, value, combiMixStyle);
        // Set to the first 8 of 24 bits.
        channels[_firstChannel.Channel()] = (mixedValue & (~0xFFFF));
        // Set to bits 9-16.
        channels[_firstChannel.Next().Channel()] = (mixedValue & 0xFFFF) << 8;
      }
    }
  }

  void SetChannel(const DmxChannel &channel, bool is16Bit = false);
  bool IsSingleChannel() const { return !_is16Bit; }
  const DmxChannel &FirstChannel() const { return _firstChannel; }
  void IncChannel();
  void DecChannel();
  FunctionType Type() const { return _type; }
  unsigned char GetCharValue(const ValueSnapshot &snapshot) const {
    return snapshot.GetValue(_firstChannel);
  }
  unsigned GetControlValue(const ValueSnapshot &snapshot) const {
    if (_is16Bit)
      return (unsigned(snapshot.GetValue(_firstChannel)) << 16) +
             (unsigned(snapshot.GetValue(_firstChannel.Next())) << 8);
    else
      return unsigned(snapshot.GetValue(_firstChannel)) << 16;
  }

 private:
  Theatre &_theatre;
  FunctionType _type;
  DmxChannel _firstChannel;
  bool _is16Bit;
};

}  // namespace glight::theatre

#endif
