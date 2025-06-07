#include "instance.h"

#include "system/settings.h"

namespace glight::gui {

Instance::Instance() { settings_ = std::make_unique<system::Settings>(); }

Instance::~Instance() = default;

}  // namespace glight::gui
