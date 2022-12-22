#ifndef THEATRE_FIXTURE_CONTROL_H_
#define THEATRE_FIXTURE_CONTROL_H_

#include "controllable.h"
#include "fixture.h"

namespace glight::theatre {

class FixtureControl final : public Controllable {
 public:
  FixtureControl(Fixture &fixture)
      : Controllable(fixture.Name()),
        _fixture(&fixture),
        _values(fixture.Functions().size()) {}

  Fixture &GetFixture() const { return *_fixture; }

  size_t NInputs() const override { return _fixture->Functions().size(); }

  ControlValue &InputValue(size_t index) override { return _values[index]; }

  virtual FunctionType InputType(size_t index) const override {
    return _fixture->Functions()[index]->Type();
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
      case FT::Rotation:
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
    return std::pair<const Controllable *, size_t>(nullptr, 0);
  }

  void Mix(const Timing &, bool) override {}

  void MixChannels(unsigned *channelValues, unsigned universe) {
    for (size_t i = 0; i != _fixture->Functions().size(); ++i) {
      const std::unique_ptr<FixtureFunction> &ff = _fixture->Functions()[i];
      ff->MixChannels(_values[i].UInt(), MixStyle::Default, channelValues,
                      universe);
    }
  }

 private:
  Fixture *_fixture;
  std::vector<ControlValue> _values;
};

}  // namespace glight::theatre

#endif
