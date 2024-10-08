#ifndef THEATRE_FIXTURE_CONTROL_H_
#define THEATRE_FIXTURE_CONTROL_H_

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
      : Controllable(fixture.Name()),
        fixture_(&fixture),
        values_(fixture.Functions().size()) {}

  Fixture &GetFixture() const { return *fixture_; }

  size_t NInputs() const override {
    if (filters_.empty())
      return fixture_->Functions().size();
    else
      return filters_.back()->InputTypes().size();
  }

  ControlValue &InputValue(size_t index) override { return values_[index]; }

  virtual FunctionType InputType(size_t index) const override {
    if (filters_.empty())
      return fixture_->Type().Functions()[index].Type();
    else
      return filters_.back()->InputTypes()[index].Type();
  }

  Color InputColor(size_t index) const {
    return GetFunctionColor(InputType(index));
  }

  virtual std::vector<Color> InputColors(size_t index) const override {
    return {InputColor(index)};
  }

  size_t NOutputs() const override { return 0; }

  std::pair<const Controllable *, size_t> Output(size_t) const override {
    assert(false);
    return std::pair<const Controllable *, size_t>(nullptr, 0);
  }

  void Mix(const Timing &, bool is_primary) override {
    // Propagate control values through the filters
    for (auto iterator = filters_.rbegin(); iterator != filters_.rend();
         ++iterator) {
      std::unique_ptr<Filter> &filter = *iterator;
      scratch_.resize(filter->OutputTypes().size());
      values_.resize(filter->InputTypes().size());
      filter->Apply(values_, scratch_);
      std::swap(scratch_, values_);
    }
    values_.resize(NInputs());
  }

  void GetChannelValues(unsigned *channelValues, unsigned universe) const {
    for (size_t i = 0; i != fixture_->Functions().size(); ++i) {
      const std::unique_ptr<FixtureFunction> &ff = fixture_->Functions()[i];
      ff->MixChannels(values_[i].UInt(), MixStyle::Default, channelValues,
                      universe);
    }
  }

  /**
   * Add a filter in front of the already filtered fixture.
   */
  void AddFilter(std::unique_ptr<Filter> &&filter) {
    assert(filter);
    if (filters_.empty()) {
      filters_.emplace_back(std::move(filter));
      filters_.back()->SetOutputTypes(fixture_->Type().Functions());
    } else {
      Filter *previous_last = filters_.back().get();
      filters_.emplace_back(std::move(filter));
      filters_.back()->SetOutputTypes(previous_last->InputTypes());
    }
    values_.resize(NInputs());
  }

  const std::vector<std::unique_ptr<Filter>> &Filters() const {
    return filters_;
  }

 private:
  Fixture *fixture_;
  std::vector<ControlValue> values_;
  std::vector<ControlValue> scratch_;
  // The filters, in backward order. Therefore, filters_.back()
  // defines the inputs of this fixture, and the result of filters_.back()
  // is sent to the previous filter, unless filters_.front() is reached.
  std::vector<std::unique_ptr<Filter>> filters_;
};

}  // namespace glight::theatre

#endif
