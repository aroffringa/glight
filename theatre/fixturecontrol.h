#ifndef FIXTURE_CONTROL_H
#define FIXTURE_CONTROL_H

#include "controllable.h"
#include "fixture.h"

class FixtureControl : public Controllable {
 public:
  FixtureControl(class Fixture &fixture)
      : Controllable(fixture.Name()),
        _fixture(&fixture),
        _values(fixture.Functions().size()) {}

  class Fixture &Fixture() const {
    return *_fixture;
  }

  size_t NInputs() const final override { return _fixture->Functions().size(); }

  ControlValue &InputValue(size_t index) final override {
    return _values[index];
  }

  virtual FunctionType InputType(size_t index) const final override {
    return _fixture->Functions()[index]->Type();
  }

  virtual Color InputColor(size_t index) const final override {
    switch (InputType(index)) {
      case FunctionType::Master:
      case FunctionType::White:
      case FunctionType::ColorMacro:
      case FunctionType::Strobe:
      case FunctionType::Pulse:
      case FunctionType::Rotation:
      case FunctionType::Pan:
      case FunctionType::Tilt:
      case FunctionType::Effect:
        return Color::White();
      case FunctionType::Red:
        return Color::RedC();
      case FunctionType::Green:
        return Color::GreenC();
      case FunctionType::Blue:
        return Color::BlueC();
      case FunctionType::Amber:
        return Color::Amber();
      case FunctionType::UV:
        return Color::UV();
      case FunctionType::Lime:
        return Color::Lime();
      case FunctionType::ColdWhite:
        return Color::ColdWhite();
      case FunctionType::WarmWhite:
        return Color::WarmWhite();
    }
    return Color::Black();
  }

  size_t NOutputs() const final override { return 0; }

  std::pair<Controllable *, size_t> Output(size_t) const final override {
    return std::pair<Controllable *, size_t>(nullptr, 0);
  }

  void Mix(const class Timing &) final override {}

  void MixChannels(unsigned *channelValues, unsigned universe) {
    for (size_t i = 0; i != _fixture->Functions().size(); ++i) {
      const std::unique_ptr<FixtureFunction> &ff = _fixture->Functions()[i];
      ff->MixChannels(_values[i].UInt(), ControlValue::Default, channelValues,
                      universe);
    }
  }

 private:
  class Fixture *_fixture;
  std::vector<ControlValue> _values;
};

#endif
