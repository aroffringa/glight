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
    functions_.emplace_back(std::make_unique<FixtureFunction>(name));
    const DmxChannel main_channel(base_channel + function.DmxOffset());
    std::optional<DmxChannel> fine_channel;
    if (function.FineChannelOffset())
      fine_channel = base_channel + *function.FineChannelOffset();

    functions_[ci]->SetChannel(main_channel, fine_channel);
  }
}

void Fixture::IncChannel() {
  for (std::unique_ptr<FixtureFunction> &ff : functions_) ff->IncChannel();

  theatre_.NotifyDmxChange();
}

void Fixture::DecChannel() {
  for (std::unique_ptr<FixtureFunction> &ff : functions_) ff->DecChannel();

  theatre_.NotifyDmxChange();
}

DmxChannel Fixture::GetFirstChannel() const {
  return functions_.front()->MainChannel();
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
