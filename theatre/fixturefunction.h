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
 * This class holds the dmx information and name of a fixture's function.
 * Objects of this class are matched with a FixtureTypeFunction, which can
 * be found through the Fixture class: function index i of a Fixture matches
 * with FixtureTypeFunction i of the corresponding FixtureType.
 */
class FixtureFunction final : public NamedObject {
 public:
  FixtureFunction() = default;

  FixtureFunction(const std::string &name);

  FixtureFunction(const FixtureFunction &source) = default;
  FixtureFunction &operator=(const FixtureFunction &source) = default;

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

  const std::optional<DmxChannel> &FineChannel() const { return fine_channel_; }
  const DmxChannel &MainChannel() const { return main_channel_; }

  /** The caller must call theatre.NotifyDmxChange(); afterward. */
  void SetChannel(const DmxChannel &channel,
                  const std::optional<DmxChannel> &fine_channel = {});
  /** The caller must call theatre.NotifyDmxChange(); afterward. */
  void IncChannel();
  /** The caller must call theatre.NotifyDmxChange(); afterward. */
  void DecChannel();
  /** The caller must call theatre.NotifyDmxChange(); afterward. */
  void SetUniverse(unsigned universe) {
    main_channel_.SetUniverse(universe);
    if (fine_channel_) fine_channel_->SetUniverse(universe);
  }

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
  DmxChannel main_channel_{0, 0};
  std::optional<DmxChannel> fine_channel_{};
};

}  // namespace glight::theatre

#endif
