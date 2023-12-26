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
      return fixture_->Functions()[index]->Type();
    else
      return filters_.back()->InputTypes()[index];
  }

  Color InputColor(size_t index) const {
    using FT = FunctionType;
    switch (InputType(index)) {
      case FT::ColorMacro:
      case FT::ColorTemperature:
      case FT::Effect:
      case FT::Lightness:
      case FT::Master:
      case FT::Pan:
      case FT::Pulse:
      case FT::RotationSpeed:
      case FT::Saturation:
      case FT::Strobe:
      case FT::Tilt:
      case FT::Zoom:
      case FT::Unknown:
      case FT::White:
        return Color::White();
      case FT::Hue:
      case FT::Red:
        return Color::RedC();
      case FT::Green:
        return Color::GreenC();
      case FT::Blue:
        return Color::BlueC();
      case FT::Amber:
        return Color::Amber();
      case FT::UV:
        return Color::UV();
      case FT::Lime:
        return Color::Lime();
      case FT::ColdWhite:
        return Color::ColdWhite();
      case FT::WarmWhite:
        return Color::WarmWhite();
    }
    return Color::Black();
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
    for (std::unique_ptr<Filter> &filter : filters_) {
      scratch_.resize(filter->OutputTypes().size());
      filter->Apply(values_, scratch_);
      std::swap(scratch_, values_);
    }
  }

  void GetChannelValues(unsigned *channelValues, unsigned universe) const {
    for (size_t i = 0; i != fixture_->Functions().size(); ++i) {
      const std::unique_ptr<FixtureFunction> &ff = fixture_->Functions()[i];
      ff->MixChannels(values_[i].UInt(), MixStyle::Default, channelValues,
                      universe);
    }
  }

  void AddFilter(std::unique_ptr<Filter> &&filter) {
    if (filters_.empty()) {
      filters_.emplace_back(std::move(filter));
      std::vector<FunctionType> types;
      types.reserve(fixture_->Functions().size());
      for (const std::unique_ptr<FixtureFunction> &function :
           fixture_->Functions()) {
        types.emplace_back(function->Type());
      }
      filters_.front()->SetOutputTypes(types);
    } else {
      filters_.insert(filters_.begin(), std::move(filter));
      filters_.front()->SetOutputTypes(filters_[1]->InputTypes());
    }
  }

  const std::vector<std::unique_ptr<Filter>> &Filters() { return filters_; }

 private:
  Fixture *fixture_;
  std::vector<ControlValue> values_;
  std::vector<ControlValue> scratch_;
  std::vector<std::unique_ptr<Filter>> filters_;
};

}  // namespace glight::theatre

#endif
