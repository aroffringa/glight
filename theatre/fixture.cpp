#include "fixture.h"
#include "theatre.h"

Fixture::Fixture(Theatre &theatre, const FixtureType &type,
                 const std::string &name)
    : NamedObject(name), _theatre(theatre), _type(type) {
  size_t ch = theatre.FirstFreeChannel();
  for (size_t ci = 0; ci != type.FunctionTypes().size(); ++ci) {
    const FunctionType functionType = type.FunctionTypes()[ci];
    const std::string name(AbbreviatedFunctionType(functionType));
    _functions.emplace_back(
        std::make_unique<FixtureFunction>(_theatre, functionType, name));
    const bool is16bit = type.Is16Bit(ci);
    _functions[ci]->SetChannel(DmxChannel(ch % 512, 0), is16bit);
    ch += is16bit ? 2 : 1;
  }
}

Fixture::Fixture(const Fixture &source, class Theatre &theatre)
    : NamedObject(source),
      _theatre(theatre),
      _type(theatre.GetFixtureType(source._type.Name())),
      _position(source._position),
      _symbol(source._symbol) {
  for (const std::unique_ptr<FixtureFunction> &ff : source._functions)
    _functions.emplace_back(new FixtureFunction(*ff, theatre));
}

void Fixture::IncChannel() {
  for (std::unique_ptr<FixtureFunction> &ff : _functions) ff->IncChannel();

  _theatre.NotifyDmxChange();
}

void Fixture::DecChannel() {
  for (std::unique_ptr<FixtureFunction> &ff : _functions) ff->DecChannel();

  _theatre.NotifyDmxChange();
}

void Fixture::SetChannel(unsigned dmxChannel) {
  for (std::unique_ptr<FixtureFunction> &ff : _functions) {
    dmxChannel = dmxChannel % 512;
    DmxChannel c = ff->FirstChannel();
    c.SetChannel(dmxChannel);
    ff->SetChannel(c);
    if (ff->IsSingleChannel())
      ++dmxChannel;
    else
      dmxChannel += 2;
  }

  _theatre.NotifyDmxChange();
}
