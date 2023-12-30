#include "theatre/colordeduction.h"

#include <boost/test/unit_test.hpp>

using glight::theatre::Color;
using glight::theatre::ControlValue;
using glight::theatre::Solve3ColorFit;

namespace {

unsigned Residual(ControlValue value, ControlValue result[2],
                  unsigned char color_a_value, unsigned char color_b_value) {
  const unsigned term =
      (result[0].UInt() * color_a_value + result[1].UInt() * color_b_value) /
      255;
  if (value.UInt() >= term)
    return value.UInt() - term;
  else
    return term - value.UInt();
}

/**
 * Tests if the fit of color_a and color_b to rgb produces
 * a fit with (almost) no residual.
 */
void Fit2ResidualTest(Color color_a, Color color_b, const ControlValue rgb[3]) {
  ControlValue truncated_rgb[3];
  for (size_t i = 0; i != 3; ++i)
    truncated_rgb[i] = Min(ControlValue::Max(), rgb[i]);
  ControlValue result[2];
  Solve2ColorFit(color_a, color_b, rgb, result);
  BOOST_CHECK_LE(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_LE(result[1].UInt(), ControlValue::MaxUInt());

  const unsigned tolerance = 256;
  const unsigned residual_r =
      Residual(truncated_rgb[0], result, color_a.Red(), color_b.Red());
  BOOST_CHECK_LT(residual_r, tolerance);

  const unsigned residual_g =
      Residual(truncated_rgb[1], result, color_a.Green(), color_b.Green());
  BOOST_CHECK_LT(residual_g, tolerance);

  const unsigned residual_b =
      Residual(truncated_rgb[2], result, color_a.Blue(), color_b.Blue());
  BOOST_CHECK_LT(residual_b, tolerance);
}

void Fit3ResidualTest(Color color_a, Color color_b, Color color_c,
                      const ControlValue rgb[3]) {
  ControlValue truncated_rgb[3];
  for (size_t i = 0; i != 3; ++i)
    truncated_rgb[i] = Min(ControlValue::Max(), rgb[i]);

  ControlValue result[3];
  Solve3ColorFit(color_a, color_b, color_c, rgb, result);

  const unsigned tolerance = 256;
  const unsigned residual_r =
      truncated_rgb[0].UInt() -
      (result[0].UInt() * color_a.Red() + result[1].UInt() * color_b.Red() +
       result[2].UInt() * color_c.Red()) /
          255;
  BOOST_CHECK_LT(residual_r, tolerance);

  const unsigned residual_g =
      truncated_rgb[1].UInt() -
      (result[0].UInt() * color_a.Green() + result[1].UInt() * color_b.Green() +
       result[2].UInt() * color_c.Green()) /
          255;
  BOOST_CHECK_LT(residual_g, tolerance);

  const unsigned residual_b =
      truncated_rgb[2].UInt() -
      (result[0].UInt() * color_a.Blue() + result[1].UInt() * color_b.Blue() +
       result[2].UInt() * color_c.Blue()) /
          255;
  BOOST_CHECK_LT(residual_b, tolerance);
}

}  // namespace

BOOST_AUTO_TEST_SUITE(color_deduction)

BOOST_AUTO_TEST_CASE(fit_2_colors_full) {
  const ControlValue rgb[3] = {ControlValue::Max(), ControlValue::Max(),
                               ControlValue::Max()};
  ControlValue result[2];

  Solve2ColorFit(Color::RedC(), Color::GreenC(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[1].UInt(), ControlValue::MaxUInt());

  Solve2ColorFit(Color::GreenC(), Color::RedC(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[1].UInt(), ControlValue::MaxUInt());

  Solve2ColorFit(Color::RedC(), Color::BlueC(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[1].UInt(), ControlValue::MaxUInt());

  Solve2ColorFit(Color::GreenC(), Color::BlueC(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[1].UInt(), ControlValue::MaxUInt());

  Solve2ColorFit(Color::Yellow(), Color::BlueC(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[1].UInt(), ControlValue::MaxUInt());

  Solve2ColorFit(Color::Purple(), Color::GreenC(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[1].UInt(), ControlValue::MaxUInt());

  Solve2ColorFit(Color::Cyan(), Color::RedC(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[1].UInt(), ControlValue::MaxUInt());

  Solve2ColorFit(Color::White(), Color::Black(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[1].UInt(), ControlValue::MaxUInt());
}

BOOST_AUTO_TEST_CASE(fit_2_colors_complex_a) {
  const ControlValue rgb[3] = {ControlValue::Max(), ControlValue::Max() / 4,
                               ControlValue::Zero()};
  constexpr Color amber(255, 128, 0);
  Fit2ResidualTest(Color::RedC(), amber, rgb);
  Fit2ResidualTest(amber, Color::RedC(), rgb);
  Fit2ResidualTest(Color::RedC(), Color::Yellow(), rgb);
  Fit2ResidualTest(Color::Yellow(), Color::RedC(), rgb);
  Fit2ResidualTest(Color::RedC(), Color::GreenC(), rgb);
  Fit2ResidualTest(Color::GreenC(), Color::RedC(), rgb);
}

BOOST_AUTO_TEST_CASE(solve_2_color_fit_complex_b) {
  const ControlValue rgb[3] = {ControlValue::Max(), ControlValue::Max(),
                               ControlValue::Max() / 2};
  constexpr Color yellow(128, 128, 0);
  constexpr Color gray(128, 128, 128);
  Fit2ResidualTest(yellow, gray, rgb);
  Fit2ResidualTest(gray, yellow, rgb);
}

BOOST_AUTO_TEST_CASE(fit_2_colors_oversaturated) {
  const ControlValue rgb[3] = {ControlValue::Max(), ControlValue::Max(),
                               ControlValue::Zero()};
  constexpr Color yellowish(128, 255, 0);
  Fit2ResidualTest(Color::RedC(), yellowish, rgb);
  // Fit2ResidualTest(yellowish, Color::RedC(), rgb);
}

BOOST_AUTO_TEST_CASE(solve_3_color_fit_case_a) {
  const ControlValue rgb[3] = {ControlValue::Max(), ControlValue::Max(),
                               ControlValue::Max()};
  ControlValue result[3];

  Solve3ColorFit(Color::RedC(), Color::GreenC(), Color::BlueC(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[1].UInt(), ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(result[2].UInt(), ControlValue::MaxUInt());

  Solve3ColorFit(Color::RedC(), Color::Amber(), Color::Yellow(), rgb, result);
  BOOST_CHECK_EQUAL(result[0].UInt(), 0);
  BOOST_CHECK_EQUAL(result[1].UInt(), 0);
  BOOST_CHECK_EQUAL(result[2].UInt(), ControlValue::MaxUInt());
}

BOOST_AUTO_TEST_CASE(solve_3_color_fit_case_b) {
  const ControlValue rgb[3] = {ControlValue::Max(), ControlValue::Max() / 4,
                               ControlValue::Zero()};
  ControlValue result[3];
  constexpr Color color_a(Color::RedC());
  constexpr Color color_b(Color::Amber());
  constexpr Color color_c(Color::Yellow());
  Solve3ColorFit(color_a, color_b, color_c, rgb, result);
  BOOST_CHECK_LT(result[0].UInt(), ControlValue::MaxUInt() * 51 / 100);
  BOOST_CHECK_GT(result[0].UInt(), ControlValue::MaxUInt() * 49 / 100);
  BOOST_CHECK_LT(result[1].UInt(), ControlValue::MaxUInt() * 51 / 100);
  BOOST_CHECK_GT(result[1].UInt(), ControlValue::MaxUInt() * 49 / 100);
  BOOST_CHECK_LT(result[2].UInt(), 256);

  Fit3ResidualTest(color_a, color_b, color_c, rgb);
}

BOOST_AUTO_TEST_CASE(solve_3_color_fit_case_c) {
  const ControlValue rgb[3] = {ControlValue::Max() / 3, ControlValue::Max() / 3,
                               ControlValue::Zero()};
  Fit3ResidualTest(Color::RedC(), Color::Amber(), Color::Yellow(), rgb);
  Fit3ResidualTest(Color::Amber(), Color::Yellow(), Color::RedC(), rgb);
  Fit3ResidualTest(Color::Yellow(), Color::RedC(), Color::Amber(), rgb);
}

BOOST_AUTO_TEST_SUITE_END()
