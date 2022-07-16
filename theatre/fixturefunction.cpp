#include "fixturefunction.h"
#include "theatre.h"

namespace glight::theatre {

FixtureFunction::FixtureFunction(Theatre &theatre, FunctionType type,
                                 const std::string &name)
    : NamedObject(name),
      _theatre(theatre),
      _type(type),
      _firstChannel(0, 0),
      _is16Bit(false) {}

FixtureFunction::FixtureFunction(Theatre &theatre, FunctionType type)
    : NamedObject(),
      _theatre(theatre),
      _type(type),
      _firstChannel(0, 0),
      _is16Bit(false) {}

FixtureFunction::FixtureFunction(const FixtureFunction &source,
                                 class Theatre &theatre)
    : NamedObject(source),
      _theatre(theatre),
      _type(source._type),
      _firstChannel(source._firstChannel),
      _is16Bit(source._is16Bit) {}

void FixtureFunction::IncChannel() {
  _firstChannel =
      DmxChannel((_firstChannel.Channel() + 1) % 512, _firstChannel.Universe());
  _theatre.NotifyDmxChange();
}

void FixtureFunction::DecChannel() {
  _firstChannel = DmxChannel((_firstChannel.Channel() + 512 - 1) % 512,
                             _firstChannel.Universe());
  _theatre.NotifyDmxChange();
}

void FixtureFunction::SetChannel(const DmxChannel &channel, bool is16Bit) {
  _firstChannel = channel;
  _is16Bit = is16Bit;
  _theatre.NotifyDmxChange();
}

}  // namespace glight::theatre
