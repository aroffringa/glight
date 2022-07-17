#include "fixture.h"
#include "theatre.h"

namespace glight::theatre {

Fixture::Fixture(Theatre &theatre, const FixtureType &type,
                 const std::string &name)
    : NamedObject(name), _theatre(theatre), _type(type) {
  size_t ch = theatre.FirstFreeChannel();
  for (size_t ci = 0; ci != type.Functions().size(); ++ci) {
    const FixtureTypeFunction function = type.Functions()[ci];
    const std::string name(AbbreviatedFunctionType(function.Type()));
    _functions.emplace_back(
        std::make_unique<FixtureFunction>(_theatre, function.Type(), name));
    const bool is16bit = function.Is16Bit();
    _functions[ci]->SetChannel(DmxChannel((ch + function.DmxOffset()) % 512, 0),
                               is16bit);
  }
}

Fixture::Fixture(const Fixture &source, Theatre &theatre)
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

}  // namespace glight::theatre
