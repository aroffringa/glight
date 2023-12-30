#include "colordeduction.h"

#include <algorithm>
#include <iostream>  // DEBUG

namespace glight::theatre {

namespace {
unsigned FitColor(Color color, unsigned red, unsigned green, unsigned blue) {
  const unsigned cr =
      color.Red() == 0 ? ControlValue::MaxUInt() : 255u * red / color.Red();
  const unsigned cg = color.Green() == 0 ? ControlValue::MaxUInt()
                                         : 255u * green / color.Green();
  const unsigned cb =
      color.Blue() == 0 ? ControlValue::MaxUInt() : 255u * blue / color.Blue();
  return std::min({ControlValue::MaxUInt(), cr, cg, cb});
}

// This returns an unnormalized value, only used for comparison
constexpr unsigned Mismatch(Color color, unsigned fit, unsigned red,
                            unsigned green, unsigned blue) {
  return (red - fit * color.Red()) + (green - fit * color.Green()) +
         (blue - fit * color.Blue());
}

void Add(Color color, unsigned fit, unsigned& red, unsigned& green,
         unsigned& blue) {
  red += fit * color.Red() / 255;
  green += fit * color.Green() / 255;
  blue += fit * color.Blue() / 255;
}

void Subtract(Color color, unsigned fit, unsigned& red, unsigned& green,
              unsigned& blue) {
  const unsigned red_term = fit * color.Red() / 255;
  red = std::max(red_term, red) - red_term;
  const unsigned green_term = fit * color.Green() / 255;
  green = std::max(green_term, green) - green_term;
  const unsigned blue_term = fit * color.Blue() / 255;
  blue = std::max(blue_term, blue) - blue_term;
}

unsigned DiffFit(Color a, Color b, unsigned red, unsigned green, unsigned blue,
                 unsigned fit_a, unsigned fit_b) {
  const unsigned char diff_r =
      std::max(0, static_cast<int>(b.Red()) - static_cast<int>(a.Red()));
  const unsigned char diff_g =
      std::max(0, static_cast<int>(b.Green()) - static_cast<int>(a.Green()));
  const unsigned char diff_b =
      std::max(0, static_cast<int>(b.Blue()) - static_cast<int>(a.Blue()));
  const Color diff_color(Color(diff_r, diff_g, diff_b));
  unsigned residual = red + green + blue;
  unsigned fit_diff = FitColor(diff_color, red, green, blue);
  fit_diff = std::clamp(fit_diff, 0u,
                        std::min(fit_a, ControlValue::MaxUInt() - fit_b));
  Add(a, fit_diff, red, green, blue);
  Subtract(b, fit_diff, red, green, blue);
  if (residual <= red + green + blue)
    return 0;
  else
    return fit_diff;
}

void ApplyDiffFit(Color a, Color b, unsigned& red, unsigned& green,
                  unsigned& blue, unsigned& fit_a, unsigned& fit_b) {
  const unsigned fit_diff = DiffFit(a, b, red, green, blue, fit_a, fit_b);
  Add(a, fit_diff, red, green, blue);
  fit_a -= fit_diff;
  Subtract(b, fit_diff, red, green, blue);
  fit_b += fit_diff;
  std::cout << "Fit_diff=" << fit_diff << ", rgb=" << red << "," << green << ","
            << blue << '\n';
}

}  // namespace

void Solve2ColorFit(Color a, Color b, const ControlValue* rgb_values,
                    ControlValue* fitted_values) {
  unsigned red = std::min(rgb_values[0].UInt(), ControlValue::MaxUInt());
  unsigned green = std::min(rgb_values[1].UInt(), ControlValue::MaxUInt());
  unsigned blue = std::min(rgb_values[2].UInt(), ControlValue::MaxUInt());

  unsigned fit_a = FitColor(a, red, green, blue);
  Subtract(a, fit_a, red, green, blue);
  unsigned fit_b = FitColor(b, red, green, blue);
  Subtract(b, fit_b, red, green, blue);

  ApplyDiffFit(a, b, red, green, blue, fit_a, fit_b);
  std::cout << "Before=" << fit_a << '\n';
  fit_a =
      std::min(ControlValue::MaxUInt(), fit_a + FitColor(a, red, green, blue));
  std::cout << "After=" << fit_a << ", RGB=" << red << ',' << green << ','
            << blue << '\n';

  fitted_values[0] = ControlValue(fit_a);
  fitted_values[1] = ControlValue(fit_b);
}

void Solve3ColorFit(Color a, Color b, Color c, const ControlValue* rgb_values,
                    ControlValue* fitted_values) {
  unsigned red = std::min(rgb_values[0].UInt(), ControlValue::MaxUInt());
  unsigned green = std::min(rgb_values[1].UInt(), ControlValue::MaxUInt());
  unsigned blue = std::min(rgb_values[2].UInt(), ControlValue::MaxUInt());

  unsigned fit_a = FitColor(a, red, green, blue);
  Subtract(a, fit_a, red, green, blue);
  std::cout << "fit_a=" << fit_a << ", rgb=" << red << "," << green << ","
            << blue << '\n';
  unsigned fit_b = FitColor(b, red, green, blue);
  Subtract(b, fit_b, red, green, blue);
  std::cout << "fit_b=" << fit_b << ", rgb=" << red << "," << green << ","
            << blue << '\n';
  unsigned fit_c = FitColor(c, red, green, blue);
  Subtract(c, fit_c, red, green, blue);
  std::cout << "fit_c=" << fit_c << ", rgb=" << red << "," << green << ","
            << blue << '\n';

  ApplyDiffFit(a, b, red, green, blue, fit_a, fit_b);
  std::cout << "fit_a=" << fit_a << ", rgb=" << red << "," << green << ","
            << blue << '\n';
  ApplyDiffFit(a, c, red, green, blue, fit_a, fit_c);
  std::cout << "fit_a=" << fit_a << ", rgb=" << red << "," << green << ","
            << blue << '\n';
  ApplyDiffFit(b, c, red, green, blue, fit_b, fit_c);
  std::cout << "fit_a=" << fit_a << ", rgb=" << red << "," << green << ","
            << blue << '\n';
  ApplyDiffFit(b, a, red, green, blue, fit_b, fit_a);
  std::cout << "fit_a=" << fit_a << ", rgb=" << red << "," << green << ","
            << blue << '\n';
  ApplyDiffFit(c, a, red, green, blue, fit_c, fit_a);
  std::cout << "fit_a=" << fit_a << ", rgb=" << red << "," << green << ","
            << blue << '\n';
  ApplyDiffFit(c, b, red, green, blue, fit_c, fit_b);

  fitted_values[0] = ControlValue(fit_a);
  fitted_values[1] = ControlValue(fit_b);
  fitted_values[2] = ControlValue(fit_c);
}

void Normalized3ColorFit(Color a, Color b, Color c,
                         const ControlValue* rgb_values,
                         ControlValue* fitted_values) {
  const unsigned m = std::max(
      {rgb_values[0].UInt(), rgb_values[1].UInt(), rgb_values[2].UInt()});
  if (m == 0) {
    std::fill_n(fitted_values, 3, ControlValue(0));
  } else if (m != ControlValue::MaxUInt()) {
    const double ratio =
        static_cast<double>(ControlValue::MaxUInt()) / static_cast<double>(m);
    const ControlValue normalized_rgb[3] = {
        rgb_values[0] * ratio, rgb_values[1] * ratio, rgb_values[2] * ratio};
    Solve3ColorFit(a, b, c, normalized_rgb, fitted_values);
    const ControlValue max_fit =
        Max(fitted_values[0], fitted_values[1], fitted_values[2]);

    if (max_fit.UInt() == 0) {
      std::fill_n(fitted_values, 3, ControlValue(0));
    } else {
      Solve3ColorFit(a, b, c, rgb_values, fitted_values);
      const double factor =
          static_cast<double>(max_fit.UInt()) /
          static_cast<double>(
              Max(fitted_values[0], fitted_values[1], fitted_values[2]).UInt());
      for (size_t i = 0; i != 3; ++i) {
        fitted_values[i] = ControlValue(
            std::min(ControlValue::MaxUInt(),
                     static_cast<unsigned>(factor * fitted_values[i].UInt())));
      }
    }
  }
}

}  // namespace glight::theatre
