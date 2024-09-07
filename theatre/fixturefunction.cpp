#include "fixturefunction.h"
#include "theatre.h"

namespace glight::theatre {

FixtureFunction::FixtureFunction(const std::string &name) : NamedObject(name) {}

void FixtureFunction::IncChannel() {
  main_channel_ = main_channel_.Next();
  if (fine_channel_) fine_channel_ = fine_channel_->Next();
}

void FixtureFunction::DecChannel() {
  main_channel_ = main_channel_.Previous();
  if (fine_channel_) fine_channel_ = fine_channel_->Previous();
}

void FixtureFunction::SetChannel(
    const DmxChannel &channel, const std::optional<DmxChannel> &fine_channel) {
  main_channel_ = channel;
  fine_channel_ = fine_channel;
}

}  // namespace glight::theatre
