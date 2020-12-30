#include "fixturefunction.h"
#include "theatre.h"

FixtureFunction::FixtureFunction(Theatre &theatre, FunctionType type,
                                 const std::string &name)
    : NamedObject(name), _theatre(theatre), _type(type), _firstChannel(0, 0) {}

FixtureFunction::FixtureFunction(Theatre &theatre, FunctionType type)
    : NamedObject(), _theatre(theatre), _type(type), _firstChannel(0, 0) {}

FixtureFunction::FixtureFunction(const FixtureFunction &source,
                                 class Theatre &theatre)
    : NamedObject(source),
      _theatre(theatre),
      _type(source._type),
      _firstChannel(source._firstChannel) {}

void FixtureFunction::IncChannel() {
  if (IsSingleChannel()) {
    _firstChannel = DmxChannel((_firstChannel.Channel() + 1) % 512,
                               _firstChannel.Universe());
  }
  _theatre.NotifyDmxChange();
}

void FixtureFunction::DecChannel() {
  if (IsSingleChannel()) {
    _firstChannel = DmxChannel((_firstChannel.Channel() + 512 - 1) % 512,
                               _firstChannel.Universe());
  }
  _theatre.NotifyDmxChange();
}

void FixtureFunction::SetChannel(const DmxChannel &channel) {
  _additionalChannels.clear();
  _firstChannel = channel;
  _theatre.NotifyDmxChange();
}
