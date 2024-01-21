#include "theatre/transition.h"

#include "tests/tolerance_check.h"

#include <boost/test/unit_test.hpp>

namespace glight {

using theatre::ControlValue;
using theatre::Timing;
using theatre::Transition;
using theatre::TransitionType;

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
