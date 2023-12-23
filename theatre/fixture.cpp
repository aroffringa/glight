#include "fixture.h"
#include "theatre.h"

namespace glight::theatre {

Fixture::Fixture(Theatre &theatre, const FixtureType &type,
                 const std::string &name)
    : NamedObject(name), theatre_(theatre), type_(type) {
  size_t ch = theatre.FirstFreeChannel();
  for (size_t ci = 0; ci != type.Functions().size(); ++ci) {
    const FixtureTypeFunction function = type.Functions()[ci];
    const std::string name(AbbreviatedFunctionType(function.Type()));
    functions_.emplace_back(
        std::make_unique<FixtureFunction>(theatre_, function.Type(), name));
    const bool is16bit = function.Is16Bit();
    functions_[ci]->SetChannel(DmxChannel((ch + function.DmxOffset()) % 512, 0),
                               is16bit);
  }
}

Fixture::Fixture(const Fixture &source, Theatre &theatre)
    : NamedObject(source),
      theatre_(theatre),
      type_(theatre.GetFixtureType(source.type_.Name())),
      position_(source.position_),
      symbol_(source.symbol_) {
  for (const std::unique_ptr<FixtureFunction> &ff : source.functions_)
    functions_.emplace_back(new FixtureFunction(*ff, theatre));
}

void Fixture::IncChannel() {
  for (std::unique_ptr<FixtureFunction> &ff : functions_) ff->IncChannel();

  theatre_.NotifyDmxChange();
}

void Fixture::DecChannel() {
  for (std::unique_ptr<FixtureFunction> &ff : functions_) ff->DecChannel();

  theatre_.NotifyDmxChange();
}

void Fixture::SetChannel(unsigned dmxChannel) {
  for (std::unique_ptr<FixtureFunction> &ff : functions_) {
    dmxChannel = dmxChannel % 512;
    DmxChannel c = ff->FirstChannel();
    c.SetChannel(dmxChannel);
    ff->SetChannel(c);
    if (ff->IsSingleChannel())
      ++dmxChannel;
    else
      dmxChannel += 2;
  }

  theatre_.NotifyDmxChange();
}

}  // namespace glight::theatre
