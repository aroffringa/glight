#include "theatre/effects/rgbmastereffect.h"
#include "theatre/effects/variableeffect.h"

#include "theatre/timing.h"

#include "tests/tolerance_check.h"

#include <boost/test/unit_test.hpp>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(rgb_master_effect)

BOOST_AUTO_TEST_CASE(types) {
  RgbMasterEffect effect;

  BOOST_CHECK_EQUAL(effect.InputValue(RgbMasterEffect::kRedInput, true).UInt(), 0);
  BOOST_CHECK_EQUAL(effect.InputValue(RgbMasterEffect::kGreenInput, true).UInt(), 0);
  BOOST_CHECK_EQUAL(effect.InputValue(RgbMasterEffect::kBlueInput, true).UInt(), 0);
  BOOST_CHECK_EQUAL(effect.InputValue(RgbMasterEffect::kMasterInput, true).UInt(), 0);
  BOOST_CHECK(effect.InputType(RgbMasterEffect::kRedInput) == FunctionType::Red);
  BOOST_CHECK(effect.InputType(RgbMasterEffect::kGreenInput) == FunctionType::Green);
  BOOST_CHECK(effect.InputType(RgbMasterEffect::kBlueInput) == FunctionType::Blue);
  BOOST_CHECK(effect.InputType(RgbMasterEffect::kMasterInput) == FunctionType::Master);
}

BOOST_AUTO_TEST_CASE(mix) {
  VariableEffect variable;
  RgbMasterEffect effect;
  Timing timing;
  effect.AddConnection(variable, 0);
  effect.AddConnection(variable, 1);
  effect.AddConnection(variable, 2);

  effect.Mix(timing, true);
  BOOST_CHECK_EQUAL(variable.InputValue(0, true).UInt(), 0);
  BOOST_CHECK_EQUAL(variable.InputValue(1, true).UInt(), 0);
  BOOST_CHECK_EQUAL(variable.InputValue(2, true).UInt(), 0);

  effect.InputValue(RgbMasterEffect::kGreenInput, true) = ControlValue::Max();
  effect.Mix(timing, true);
  BOOST_CHECK_EQUAL(variable.InputValue(0, true).UInt(), 0);
  BOOST_CHECK_EQUAL(variable.InputValue(1, true).UInt(), 0);
  BOOST_CHECK_EQUAL(variable.InputValue(2, true).UInt(), 0);

  effect.InputValue(RgbMasterEffect::kMasterInput, true) = ControlValue::Max();
  effect.Mix(timing, true);
  BOOST_CHECK_EQUAL(variable.InputValue(0, true).UInt(), 0);
  ToleranceCheck(variable.InputValue(1, true), ControlValue::MaxUInt(), 256);
  BOOST_CHECK_EQUAL(variable.InputValue(2, true).UInt(), 0);

  effect.InputValue(RgbMasterEffect::kMasterInput, true) = ControlValue::Max() / 2;
  effect.InputValue(RgbMasterEffect::kRedInput, true) = ControlValue::Max() / 2;
  variable.InputValue(0, true) = ControlValue::Zero();
  variable.InputValue(1, true) = ControlValue::Zero();
  variable.InputValue(2, true) = ControlValue::Zero();
  effect.Mix(timing, true);
  ToleranceCheck(variable.InputValue(0, true), ControlValue::MaxUInt() / 4, 1024);
  ToleranceCheck(variable.InputValue(1, true), ControlValue::MaxUInt() / 2, 1024);
  BOOST_CHECK_EQUAL(variable.InputValue(2, true).UInt(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
