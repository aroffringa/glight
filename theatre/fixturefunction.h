#ifndef THEATRE_FIXTUREFUNCTION_H_
#define THEATRE_FIXTUREFUNCTION_H_

#include <optional>
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
    if (main_channel_.Universe() == universe) {
      if (!fine_channel_) {
        channels[main_channel_.Channel()] = ControlValue::Mix(
            channels[main_channel_.Channel()], value, mixStyle);
      } else {  // 16 bit
        const unsigned currentValue = (channels[main_channel_.Channel()]) +
                                      (channels[fine_channel_->Channel()] >> 8);
        const unsigned mixedValue =
            ControlValue::Mix(currentValue, value, mixStyle);
        // Set to the first 8 of 24 bits.
        channels[main_channel_.Channel()] = (mixedValue & (~0xFFFF));
        // Set to bits 9-16.
        channels[fine_channel_->Channel()] = (mixedValue & 0xFFFF) << 8;
      }
    }
  }

  void SetChannel(const DmxChannel &channel,
                  const std::optional<DmxChannel> &fine_channel = {});
  const std::optional<DmxChannel> &FineChannel() const { return fine_channel_; }
  const DmxChannel &MainChannel() const { return main_channel_; }
  void IncChannel();
  void DecChannel();
  FunctionType Type() const { return type_; }
  unsigned char GetCharValue(const ValueSnapshot &snapshot) const {
    return snapshot.GetValue(main_channel_);
  }
  unsigned GetControlValue(const ValueSnapshot &snapshot) const {
    if (fine_channel_)
      return (unsigned(snapshot.GetValue(main_channel_)) << 16) +
             (unsigned(snapshot.GetValue(*fine_channel_)) << 8);
    else
      return unsigned(snapshot.GetValue(main_channel_)) << 16;
  }

 private:
  Theatre &theatre_;
  FunctionType type_;
  DmxChannel main_channel_;
  std::optional<DmxChannel> fine_channel_;
};

}  // namespace glight::theatre

#endif
