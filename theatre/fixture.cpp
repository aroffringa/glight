#include "fixture.h"
#include "theatre.h"

namespace glight::theatre {

Fixture::Fixture(Theatre &theatre, const FixtureType &type,
                 const std::string &name)
    : NamedObject(name), theatre_(theatre), type_(type) {
  DmxChannel base_channel = theatre.FirstFreeChannel();
  for (size_t ci = 0; ci != type.Functions().size(); ++ci) {
    const FixtureTypeFunction function = type.Functions()[ci];
    const std::string name(AbbreviatedFunctionType(function.Type()));
    functions_.emplace_back(
        std::make_unique<FixtureFunction>(theatre_, function.Type(), name));
    const DmxChannel main_channel(base_channel + function.DmxOffset());
    std::optional<DmxChannel> fine_channel;
    if (function.FineChannelOffset())
      fine_channel = base_channel + *function.FineChannelOffset();

    functions_[ci]->SetChannel(main_channel, fine_channel);
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

void Fixture::SetChannel(DmxChannel dmx_channel) {
  for (size_t i = 0; i != functions_.size(); ++i) {
    std::unique_ptr<FixtureFunction> &ff = functions_[i];
    const glight::theatre::FixtureTypeFunction &tf = Type().Functions()[i];
    if (tf.FineChannelOffset()) {
      ff->SetChannel(dmx_channel + tf.DmxOffset(),
                     dmx_channel + *tf.FineChannelOffset());
    } else {
      ff->SetChannel(dmx_channel + tf.DmxOffset());
    }
  }

  theatre_.NotifyDmxChange();
}

}  // namespace glight::theatre
