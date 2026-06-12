#ifndef THEATRE_FIXTURE_CONTROL_H_
#define THEATRE_FIXTURE_CONTROL_H_

#include <array>
#include <cassert>
#include <memory>
#include <vector>

#include "controllable.h"
#include "fixture.h"

#include "filters/filter.h"

namespace glight::theatre {

class FixtureControl final : public Controllable {
 public:
  FixtureControl(Fixture &fixture)
      : Controllable(fixture.Name()), fixture_(&fixture) {
     values_[false].resize(fixture.Functions().size());
     values_[true].resize(fixture.Functions().size());
     filtered_[false].resize(fixture.Functions().size());
     filtered_[true].resize(fixture.Functions().size());
  }

  Fixture &GetFixture() const { return *fixture_; }

  size_t NInputs() const override {
    if (filters_.empty())
      return fixture_->Functions().size();
    else
      return filters_.back()->InputTypes().size();
  }

  ControlValue &InputValue(size_t index, bool primary) override { return values_[primary][index]; }

  virtual FunctionType InputType(size_t index) const override {
    if (filters_.empty())
      return fixture_->Mode().Functions()[index].Type();
    else
      return filters_.back()->InputTypes()[index].Type();
  }

  Color InputColor(size_t index) const { return GetFunctionColor(InputType(index)); }

  virtual std::vector<Color> InputColors(size_t index) const override {
    return {InputColor(index)};
  }

  size_t NConnections() const override { return 0; }

  std::pair<const Controllable *, size_t> GetConnection(size_t) const override {
    assert(false);
    return std::pair<const Controllable *, size_t>(nullptr, 0);
  }

  void Mix(const Timing &, bool is_primary) override {
    // Propagate control values through the filters
    in_scratch_ = values_[is_primary];
    for (auto iterator = filters_.rbegin(); iterator != filters_.rend(); ++iterator) {
      std::unique_ptr<Filter> &filter = *iterator;
      out_scratch_.resize(filter->OutputTypes().size());
      in_scratch_.resize(filter->InputTypes().size());
      filter->Apply(in_scratch_, out_scratch_);
      std::swap(out_scratch_, in_scratch_);
    }
    std::swap(filtered_[is_primary], in_scratch_);
  }

  void GetChannelValues(unsigned *channelValues, unsigned universe, bool primary) const {
    for (size_t i = 0; i != fixture_->Functions().size(); ++i) {
      const std::unique_ptr<FixtureFunction> &ff = fixture_->Functions()[i];
      ff->MixChannels(filtered_[primary][i].UInt(), MixStyle::Default, channelValues, universe);
    }
  }

  /**
   * Add a filter in front of the already filtered fixture.
   */
  void AddFilter(std::unique_ptr<Filter> &&filter) {
    assert(filter);
    if (filters_.empty()) {
      filters_.emplace_back(std::move(filter));
      filters_.back()->SetOutputTypes(fixture_->Mode().Functions());
    } else {
      Filter *previous_last = filters_.back().get();
      filters_.emplace_back(std::move(filter));
      filters_.back()->SetOutputTypes(previous_last->InputTypes());
    }
    values_[false].resize(NInputs());
    values_[true].resize(NInputs());
    filtered_[false].resize(NInputs());
    filtered_[true].resize(NInputs());
  }

  const std::vector<std::unique_ptr<Filter>> &Filters() const { return filters_; }

 private:
  Fixture *fixture_;
  std::array<std::vector<ControlValue>, 2> values_;
  std::vector<ControlValue> in_scratch_;
  std::vector<ControlValue> out_scratch_;
  std::array<std::vector<ControlValue>, 2> filtered_;
  // The filters, in backward order. Therefore, filters_.back()
  // defines the inputs of this fixture, and the result of filters_.back()
  // is sent to the previous filter, unless filters_.front() is reached.
  std::vector<std::unique_ptr<Filter>> filters_;
};

}  // namespace glight::theatre

#endif
