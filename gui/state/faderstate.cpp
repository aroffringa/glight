#include "guistate.h"

#include "../../theatre/controllable.h"
#include "../../theatre/management.h"
#include "../../theatre/sourcevalue.h"

namespace glight::gui {

using theatre::SourceValue;

FaderState::FaderState(std::vector<SourceValue*> source_values)
    : source_values_(std::move(source_values)) {
  Connect();
}

FaderState::~FaderState() { Disconnect(); }

void FaderState::Connect() {
  source_value_deleted_connections_.clear();
  source_value_deleted_connections_.resize(source_values_.size());
  for (size_t i = 0; i != source_values_.size(); ++i) {
    if (source_values_[i]) {
      source_value_deleted_connections_[i] =
          source_values_[i]->SignalDelete().connect(
              [&]() { onPresetValueDeleted(); });
    }
  }
}

void FaderState::Disconnect() {
  for (sigc::connection& connection : source_value_deleted_connections_)
    connection.disconnect();
}

void FaderState::SetSourceValues(std::vector<SourceValue*> source_values) {
  if (source_values != source_values_) {
    Disconnect();
    source_values_ = std::move(source_values);
    Connect();
    signal_change_();
  }
}

void FaderState::onPresetValueDeleted() {
  Disconnect();
  source_values_.clear();
  signal_change_();
}

}  // namespace glight::gui
