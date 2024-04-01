#include "universemap.h"

namespace glight::theatre::devices {

void UniverseMap::Open() {
  Close();
  mappings_.clear();
  sync_ = 0;
  try {
    bool has_output = false;
    ola_ = std::make_unique<OlaConnection>();
    ola_->Open();
    const std::vector<size_t> universes = ola_->GetUniverses();
    mappings_.reserve(universes.size());
    for (size_t universe : universes) {
      switch (ola_->GetUniverseType(universe)) {
        case UniverseType::Input:
          mappings_.emplace_back(
              InputMapping{InputMappingFunction::NoFunction,
                           {},
                           system::OptionalNumber<size_t>(universe)});
          break;
        case UniverseType::Output:
          mappings_.emplace_back(
              OutputMapping{system::OptionalNumber<size_t>(universe)});
          has_output = true;
          break;
        case UniverseType::Uninitialized:
          break;
      }
    }
    if (!has_output) {
      // Add a dummy output; otherwise there's no univere to store the
      // output channel values, and e.g. the visualization won't work.
      mappings_.emplace_back(OutputMapping());
    }
  } catch (std::exception& e) {
    std::cerr << "DMX device threw exception: " << e.what() << '\n';
    std::cerr << "No DMX device found, switching to dummy output.\n";
    ola_.reset();
    mappings_.reserve(2);
    mappings_.emplace_back(OutputMapping());
    mappings_.emplace_back(
        InputMapping{InputMappingFunction::NoFunction, {}, {}});
  }
}

}  // namespace glight::theatre::devices
