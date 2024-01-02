#include "fixturefunction.h"
#include "theatre.h"

namespace glight::theatre {

FixtureFunction::FixtureFunction(Theatre &theatre, FunctionType type,
                                 const std::string &name)
    : NamedObject(name),
      theatre_(theatre),
      type_(type),
      main_channel_(0, 0),
      fine_channel_() {}

FixtureFunction::FixtureFunction(Theatre &theatre, FunctionType type)
    : theatre_(theatre), type_(type), main_channel_(0, 0), fine_channel_() {}

FixtureFunction::FixtureFunction(const FixtureFunction &source,
                                 Theatre &theatre)
    : NamedObject(source),
      theatre_(theatre),
      type_(source.type_),
      main_channel_(source.main_channel_),
      fine_channel_(source.fine_channel_) {}

void FixtureFunction::IncChannel() {
  main_channel_ = main_channel_.Next();
  if (fine_channel_) fine_channel_ = fine_channel_->Next();
  theatre_.NotifyDmxChange();
}

void FixtureFunction::DecChannel() {
  main_channel_ = main_channel_.Previous();
  if (fine_channel_) fine_channel_ = fine_channel_->Previous();
  theatre_.NotifyDmxChange();
}

void FixtureFunction::SetChannel(
    const DmxChannel &channel, const std::optional<DmxChannel> &fine_channel) {
  main_channel_ = channel;
  fine_channel_ = fine_channel;
  theatre_.NotifyDmxChange();
}

}  // namespace glight::theatre
