#include "renderengine.h"

#include "../theatre/fixture.h"
#include "../theatre/theatre.h"

namespace glight::gui {

namespace {
constexpr double GetRadius(theatre::FixtureSymbol::Symbol symbol) {
  switch (symbol) {
    case theatre::FixtureSymbol::Hidden:
      return 0.0;
    case theatre::FixtureSymbol::Small:
      return 0.3;
    case theatre::FixtureSymbol::Normal:
      return 0.4;
    case theatre::FixtureSymbol::Large:
      return 0.5;
  }
  return 0.4;
}

constexpr double GetRadiusSquared(theatre::FixtureSymbol::Symbol symbol) {
  const double r = GetRadius(symbol);
  return r * r;
}

double GetScale(const theatre::Management &management, double width,
                double height) {
  const theatre::Theatre &theatre = management.GetTheatre();
  theatre::Position extend = theatre.Extend();
  if (extend.X() == 0.0 || extend.Y() == 0.0)
    return 1.0;
  else
    return std::min(width / extend.X(), height / extend.Y());
}

double GetInvScale(const theatre::Management &management, double width,
                   double height) {
  const double sc = GetScale(management, width, height);
  if (sc == 0.0)
    return 1.0;
  else
    return 1.0 / sc;
}

struct DrawData {
  const Cairo::RefPtr<Cairo::Context> &cairo;
  const theatre::ValueSnapshot &snapshot;
  const DrawStyle &style;
  double scale;
};

void DrawFixtureBeam(const DrawData &data, const theatre::Fixture &fixture) {
  const glight::theatre::FixtureType &type = fixture.Type();
  const size_t shape_count = type.ShapeCount();
  for (size_t shape_index = 0; shape_index != shape_count; ++shape_index) {
    const theatre::Color c = fixture.GetColor(data.snapshot, shape_index);
    if (c != theatre::Color::Black() && type.MinBeamAngle() > 0.0) {
      double direction = fixture.Direction();
      if (type.CanBeamRotate()) {
        direction += type.GetPan(fixture, data.snapshot, shape_index);
      }

      const double beam_angle =
          type.CanZoom() ? type.GetZoom(fixture, data.snapshot, shape_index)
                         : type.MinBeamAngle();
      const double direction_1 = direction - beam_angle * 0.5;
      const double direction_2 = direction + beam_angle * 0.5;
      const double radius = GetRadius(fixture.Symbol().Value());
      const double beam_start_radius = radius * 1.2;
      const double beam_factor = type.MinBeamAngle() / beam_angle;
      const double beam_end_radius =
          radius * (1.2 + type.Brightness() * beam_factor);
      const double x =
          fixture.GetPosition().X() + 0.5 + data.style.xOffset / data.scale;
      const double y =
          fixture.GetPosition().Y() + 0.5 + data.style.yOffset / data.scale;
      Cairo::RefPtr<Cairo::RadialGradient> gradient =
          Cairo::RadialGradient::create(x, y, beam_start_radius, x, y,
                                        beam_end_radius);
      double r = static_cast<double>(c.Red()) / 255.0;
      double g = static_cast<double>(c.Green()) / 255.0;
      double b = static_cast<double>(c.Blue()) / 255.0;
      const double max_rgb = std::max({r, g, b});
      r /= max_rgb;
      g /= max_rgb;
      b /= max_rgb;
      gradient->add_color_stop_rgba(0.0, r, g, b, 0.5 * max_rgb);
      gradient->add_color_stop_rgba(1.0, r, g, b, 0.0);
      data.cairo->set_source(gradient);
      const double cos_1 = std::cos(direction_1);
      const double sin_1 = std::sin(direction_1);
      const double cos_2 = std::cos(direction_2);
      const double sin_2 = std::sin(direction_2);
      data.cairo->arc_negative(x, y, beam_start_radius, direction_2,
                               direction_1);
      data.cairo->line_to(x + cos_1 * beam_end_radius,
                          y + sin_1 * beam_end_radius);
      // small optimization: don't draw an extra arc when the
      // beam is narrow
      if (beam_angle > M_PI * 0.2) {
        data.cairo->arc(x, y, beam_end_radius, direction_1, direction_2);
      } else {
        data.cairo->line_to(x + cos_2 * beam_end_radius,
                            y + sin_2 * beam_end_radius);
      }
      data.cairo->line_to(x + cos_2 * beam_start_radius,
                          y + sin_2 * beam_start_radius);
      data.cairo->fill();
    }
  }
}

void DrawFixture(const DrawData &data, const theatre::Fixture &fixture,
                 FixtureState &fixture_state) {
  size_t shapeCount = fixture.Type().ShapeCount();
  for (size_t i = 0; i != shapeCount; ++i) {
    const size_t shapeIndex = shapeCount - i - 1;
    const theatre::Color c = fixture.GetColor(data.snapshot, shapeIndex);

    data.cairo->set_source_rgb(static_cast<double>(c.Red()) / 224.0 + 0.125,
                               static_cast<double>(c.Green()) / 224.0 + 0.125,
                               static_cast<double>(c.Blue()) / 224.0 + 0.125);

    const double single_radius = GetRadius(fixture.Symbol().Value());
    const double radius =
        shapeCount == 1
            ? single_radius
            : 0.33 + 0.07 * static_cast<double>(shapeIndex) / (shapeCount - 1);
    const double x =
        fixture.GetPosition().X() + 0.5 + data.style.xOffset / data.scale;
    const double y =
        fixture.GetPosition().Y() + 0.5 + data.style.yOffset / data.scale;
    data.cairo->arc(x, y, radius, 0.0, 2.0 * M_PI);
    data.cairo->fill();

    // If a fixture is continuously rotating (e.g. a disco ball light), draw a
    // rotating cross.

    const int rotation_speed =
        fixture.GetRotationSpeed(data.snapshot, shapeIndex);
    if (rotation_speed != 0) {
      const double displayed_rotation =
          M_PI * static_cast<double>(rotation_speed) * data.style.timeSince /
          (10.0 * (1U << 24U));
      fixture_state.continuous_rotation = std::fmod(
          displayed_rotation + fixture_state.continuous_rotation, M_PI);
      const double s = std::sin(fixture_state.continuous_rotation);
      const double c = std::cos(fixture_state.continuous_rotation);
      data.cairo->set_line_width(radius * 0.2);
      data.cairo->set_source_rgb(0, 0, 0);
      data.cairo->move_to(x + c * radius + data.style.xOffset / data.scale,
                          y + s * radius + data.style.yOffset / data.scale);
      data.cairo->line_to(x - c * radius + data.style.xOffset / data.scale,
                          y - s * radius + data.style.yOffset / data.scale);
      data.cairo->move_to(x + s * radius + data.style.xOffset / data.scale,
                          y - c * radius + data.style.yOffset / data.scale);
      data.cairo->line_to(x - s * radius + data.style.xOffset / data.scale,
                          y + c * radius + data.style.yOffset / data.scale);
      data.cairo->stroke();
    }
  }
}

}  // namespace

RenderEngine::RenderEngine(const theatre::Management &management)
    : management_(management) {}

void RenderEngine::DrawSnapshot(
    const Cairo::RefPtr<Cairo::Context> &cairo,
    const theatre::ValueSnapshot &snapshot, const DrawStyle &style,
    const std::vector<theatre::Fixture *> &selected_fixtures) {
  const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
      management_.GetTheatre().Fixtures();
  cairo->save();
  scale_ = GetScale(management_, style.width, style.height);
  cairo->scale(scale_, scale_);

  const DrawData draw_data{cairo, snapshot, style, scale_};

  for (const std::unique_ptr<theatre::Fixture> &fixture : fixtures) {
    if (fixture->IsVisible()) {
      DrawFixtureBeam(draw_data, *fixture);
    }
  }

  state_.resize(fixtures.size());
  for (size_t fixtureIndex = 0; fixtureIndex != fixtures.size();
       ++fixtureIndex) {
    const theatre::Fixture &fixture = *fixtures[fixtureIndex];
    if (fixture.IsVisible()) {
      FixtureState &fixture_state = state_[fixtureIndex];
      DrawFixture(draw_data, fixture, fixture_state);
    }
  }

  cairo->set_source_rgb(0.2, 0.2, 1.0);
  cairo->set_line_width(4.0 / scale_);
  for (const theatre::Fixture *f : selected_fixtures) {
    if (f->IsVisible()) {
      double rad = GetRadius(f->Symbol().Value());
      double x = f->GetPosition().X() + 0.5;
      double y = f->GetPosition().Y() + 0.5;
      cairo->arc(x + style.xOffset / scale_, y + style.yOffset / scale_, rad,
                 0.0, 2.0 * M_PI);
      cairo->stroke();
    }
  }

  cairo->restore();
}

void RenderEngine::DrawSelectionRectangle(
    const Cairo::RefPtr<Cairo::Context> &cairo, const theatre::Position &from,
    const theatre::Position &to) const {
  const std::pair<double, double> size = to - from;
  cairo->save();
  cairo->scale(scale_, scale_);
  cairo->set_line_width(2.0 / scale_);
  cairo->rectangle(from.X(), from.Y(), size.first, size.second);
  cairo->set_source_rgba(0.2, 0.2, 1.0, 0.5);
  cairo->fill_preserve();
  cairo->set_source_rgba(0.5, 0.5, 1.0, 0.8);
  cairo->stroke();
  cairo->restore();
}

theatre::Fixture *RenderEngine::FixtureAt(
    const theatre::Position &position) const {
  const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
      management_.GetTheatre().Fixtures();

  theatre::Fixture *fixture = nullptr;
  double closest = std::numeric_limits<double>::max();
  for (const std::unique_ptr<theatre::Fixture> &f : fixtures) {
    if (f->IsVisible() &&
        position.InsideRectangle(f->GetPosition(),
                                 f->GetPosition().Add(1.0, 1.0))) {
      double distanceSq =
          position.SquaredDistance(f->GetPosition().Add(0.5, 0.5));
      double radSq = GetRadiusSquared(f->Symbol().Value());
      if (distanceSq <= radSq && distanceSq < closest) {
        fixture = f.get();
        closest = distanceSq;
      }
    }
  }
  return fixture;
}

theatre::Position RenderEngine::MouseToPosition(double mouse_x, double mouse_y,
                                                double width,
                                                double height) const {
  const double sc = GetInvScale(management_, width, height);
  return theatre::Position(mouse_x, mouse_y) * sc;
}

}  // namespace glight::gui
