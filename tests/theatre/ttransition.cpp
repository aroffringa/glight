#include "theatre/effects/variableeffect.h"
#include "theatre/transition.h"

#include "tests/tolerance_check.h"

#include <boost/test/unit_test.hpp>

namespace glight {

using theatre::Connection;
using theatre::ControlValue;
using theatre::Timing;
using theatre::Transition;
using theatre::TransitionType;
using theatre::VariableEffect;

BOOST_AUTO_TEST_SUITE(transition)

BOOST_AUTO_TEST_CASE(fade_in) {
  const Transition t(500, TransitionType::Fade);

  const ControlValue start = t.InValue(0.0, Timing());
  BOOST_CHECK_EQUAL(start.UInt(), 0);

  const ControlValue mid = t.InValue(250.0, Timing());
  theatre::ToleranceCheck(mid, ControlValue::MaxUInt() / 2, 255);

  const ControlValue end = t.InValue(500.0, Timing());
  BOOST_CHECK_EQUAL(end.UInt(), ControlValue::MaxUInt());

  const ControlValue before = t.InValue(-1.0, Timing());
  BOOST_CHECK_EQUAL(before.UInt(), 0);

  const ControlValue after = t.InValue(501.0, Timing());
  BOOST_CHECK_GE(after.UInt(), ControlValue::MaxUInt());
}

BOOST_AUTO_TEST_CASE(fade_out) {
  const Transition t(500, TransitionType::Fade);

  const ControlValue start = t.OutValue(0.0, Timing());
  BOOST_CHECK_EQUAL(start.UInt(), ControlValue::MaxUInt());

  const ControlValue mid = t.OutValue(250.0, Timing());
  theatre::ToleranceCheck(mid, ControlValue::MaxUInt() / 2, 255);

  const ControlValue end = t.OutValue(500.0, Timing());
  BOOST_CHECK_EQUAL(end.UInt(), 0);

  const ControlValue before = t.OutValue(-1.0, Timing());
  BOOST_CHECK_GE(before.UInt(), ControlValue::MaxUInt());

  const ControlValue after = t.OutValue(501.0, Timing());
  BOOST_CHECK_EQUAL(after.UInt(), 0);
}

BOOST_AUTO_TEST_CASE(fade_mix) {
  const Timing timing;
  const Transition t(500.0, TransitionType::Fade);

  VariableEffect result_a;
  Connection a0{&result_a, 0, {0, 0}};
  Connection a1{&result_a, 1, {0, 0}};
  t.Mix(a0, a1, 0.0, ControlValue::Max(), timing, true);
  BOOST_CHECK_EQUAL(result_a.InputValue(0).ToUChar(), 255);
  BOOST_CHECK_EQUAL(result_a.InputValue(1).ToUChar(), 0);

  VariableEffect result_b;
  Connection b0{&result_b, 0, {0, 0}};
  Connection b1{&result_b, 1, {0, 0}};
  t.Mix(b0, b1, 500.0, ControlValue::Max() / 2, timing, true);
  BOOST_CHECK_EQUAL(result_b.InputValue(0).ToUChar(), 0);
  BOOST_CHECK_EQUAL(result_b.InputValue(1).ToUChar(), 127);

  VariableEffect result_c;
  Connection c0{&result_c, 0, {0, 0}};
  Connection c1{&result_c, 1, {0, 0}};
  t.Mix(c0, c1, 125.0, ControlValue::Max(), timing, true);
  BOOST_CHECK_EQUAL(result_c.InputValue(0).ToUChar(), 192);
  BOOST_CHECK_EQUAL(result_c.InputValue(1).ToUChar(), 63);

  // Test for time values outside the transition range
  VariableEffect result_d;
  Connection d0{&result_d, 0, {0, 0}};
  Connection d1{&result_d, 1, {0, 0}};
  t.Mix(d0, d1, -100.0, ControlValue::Max(), timing, true);
  BOOST_CHECK_EQUAL(result_d.InputValue(0).ToUChar(), 255);
  BOOST_CHECK_EQUAL(result_d.InputValue(1).ToUChar(), 0);

  VariableEffect result_e;
  Connection e0{&result_e, 0, {0, 0}};
  Connection e1{&result_e, 1, {0, 0}};
  t.Mix(e0, e1, 600.0, ControlValue::Max(), timing, true);
  BOOST_CHECK_EQUAL(result_e.InputValue(0).ToUChar(), 0);
  BOOST_CHECK_EQUAL(result_e.InputValue(1).ToUChar(), 255);

  // Test for too high control values
  VariableEffect result_f;
  Connection f0{&result_f, 0, {0, 0}};
  Connection f1{&result_f, 1, {0, 0}};
  t.Mix(f0, f1, 500.0, ControlValue::Max() * 5u / 4, timing, true);
  BOOST_CHECK_EQUAL(result_f.InputValue(0).ToUChar(), 0);
  BOOST_CHECK_EQUAL(result_f.InputValue(1).ToUChar(), 255);
}

BOOST_AUTO_TEST_CASE(fade_through_black_in) {
  const Transition t(100, TransitionType::FadeThroughBlack);

  const ControlValue start = t.InValue(0.0, Timing());
  BOOST_CHECK_EQUAL(start.UInt(), 0);

  const ControlValue mid = t.InValue(50.0, Timing());
  BOOST_CHECK_EQUAL(mid.UInt(), 0);

  const ControlValue fading_in = t.InValue(75.0, Timing());
  theatre::ToleranceCheck(fading_in, ControlValue::MaxUInt() / 2, 255);

  const ControlValue end = t.InValue(100.0, Timing());
  BOOST_CHECK_EQUAL(end.UInt(), ControlValue::MaxUInt());
}

BOOST_AUTO_TEST_CASE(fade_through_black_out) {
  const Transition t(400, TransitionType::FadeThroughBlack);

  const ControlValue start = t.OutValue(0.0, Timing());
  BOOST_CHECK_EQUAL(start.UInt(), ControlValue::MaxUInt());

  const ControlValue fading_out = t.OutValue(100.0, Timing());
  theatre::ToleranceCheck(fading_out, ControlValue::MaxUInt() / 2, 255);

  const ControlValue mid = t.OutValue(200.0, Timing());
  BOOST_CHECK_EQUAL(mid.UInt(), 0);

  const ControlValue end = t.OutValue(400.0, Timing());
  BOOST_CHECK_EQUAL(end.UInt(), 0);
}

BOOST_AUTO_TEST_CASE(fade_through_full_in) {
  const Transition t(100, TransitionType::FadeThroughFull);

  const ControlValue start = t.InValue(0.0, Timing());
  BOOST_CHECK_EQUAL(start.UInt(), 0);

  const ControlValue fading_in = t.InValue(25.0, Timing());
  theatre::ToleranceCheck(fading_in, ControlValue::MaxUInt() / 2, 255);

  const ControlValue mid = t.InValue(50.0, Timing());
  BOOST_CHECK_EQUAL(mid.UInt(), ControlValue::MaxUInt());

  const ControlValue end = t.InValue(100.0, Timing());
  BOOST_CHECK_EQUAL(end.UInt(), ControlValue::MaxUInt());
}

BOOST_AUTO_TEST_CASE(fade_through_full_out) {
  const Transition t(800, TransitionType::FadeThroughFull);

  const ControlValue start = t.OutValue(0.0, Timing());
  BOOST_CHECK_EQUAL(start.UInt(), ControlValue::MaxUInt());

  const ControlValue mid = t.OutValue(400.0, Timing());
  BOOST_CHECK_EQUAL(mid.UInt(), ControlValue::MaxUInt());

  const ControlValue fading_out = t.OutValue(600.0, Timing());
  theatre::ToleranceCheck(fading_out, ControlValue::MaxUInt() / 2, 255);

  const ControlValue end = t.OutValue(800.0, Timing());
  BOOST_CHECK_EQUAL(end.UInt(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace glight
