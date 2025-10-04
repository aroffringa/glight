#ifndef THEATRE_DMXCHANNEL_H_
#define THEATRE_DMXCHANNEL_H_

#include <cstring>

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class DmxChannel {
 public:
  constexpr DmxChannel() : universe_(0), channel_(0) {}
  constexpr DmxChannel(unsigned channel, unsigned universe)
      : universe_(universe), channel_(channel) {}
  ~DmxChannel() = default;

  constexpr unsigned Universe() const { return universe_; }
  constexpr unsigned Channel() const { return channel_; }

  constexpr void SetUniverse(unsigned universe) { universe_ = universe; }
  constexpr void SetChannel(unsigned channel) { channel_ = channel; }
  DmxChannel Next() const {
    return DmxChannel((channel_ + 1) % 512, universe_);
  }
  DmxChannel Previous() const {
    return DmxChannel((channel_ + 512 - 1) % 512, universe_);
  }
  /**
   * Returns the DmxChannel with given offset from this channel. If the
   * resulting channel would be higher than 512, it will wrap around in the same
   * universe. The universe of the DmxChannel is always the same as this.
   */
  DmxChannel operator+(size_t channel_offset) const {
    return DmxChannel((channel_ + channel_offset) % 512, universe_);
  }

 private:
  unsigned universe_;
  unsigned channel_;
};

}  // namespace glight::theatre

#endif
