#include "renderengine.h"

#include "../theatre/fixture.h"
#include "../theatre/fixturemode.h"
#include "../theatre/fixturetype.h"
#include "../theatre/theatre.h"

#include "system/math.h"

namespace glight::gui {

namespace {

constexpr double kRotationHandleEnd = 2.0;
constexpr double kRotationHandleStart = 0.5;

constexpr double GetRadiusFactor(theatre::FixtureSymbol::Symbol symbol) {
  switch (symbol) {
    case theatre::FixtureSymbol::Hidden:
      return 0.0;
    case theatre::FixtureSymbol::Small:
      return 0.6;
    case theatre::FixtureSymbol::Normal:
      return 0.8;
    case theatre::FixtureSymbol::Large:
      return 1.0;
  }
  return 0.8;
}

double GetScale(const theatre::Management &management, double width,
                double height) {
  const theatre::Theatre &theatre = management.GetTheatre();
  theatre::Coordinate2D extend(theatre.Width(), theatre.Depth());
  constexpr double margin = 16.0;
  if (extend.X() == 0.0 || extend.Y() == 0.0)
    return 1.0;
  else
    return std::min((width - margin) / extend.X(),
                    (height - margin) / extend.Y());
}

struct DrawData {
  const Cairo::RefPtr<Cairo::Context> &cairo;
  const theatre::Management &management;
  const theatre::ValueSnapshot &snapshot;
  const DrawStyle &style;
  double scale;
  bool is_moving = false;
};

void DrawFixtureProjection(const DrawData &data,
                           const theatre::Fixture &fixture) {
  const theatre::FixtureMode &mode = fixture.Mode();
  const theatre::FixtureType &type = mode.Type();
  const size_t shape_count = type.ShapeCount();
  for (size_t shape_index = 0; shape_index != shape_count; ++shape_index) {
    const double tilt = fixture.GetBeamTilt(data.snapshot, shape_index);
    const double direction =
        fixture.GetBeamDirection(data.snapshot, shape_index);
    const double beam_angle =
        type.CanZoom() ? mode.GetZoom(fixture, data.snapshot, shape_index) * 0.5
                       : type.MinBeamAngle() * 0.5;
    const double x = fixture.GetPosition().X() + 0.5;
    const double y = fixture.GetPosition().Y() + 0.5;
    const double z = fixture.GetPosition().Z();
    const double sin_direction = std::sin(direction);
    const double cos_direction = std::cos(direction);
    if (beam_angle > 0.0 && beam_angle < M_PI) {
      const theatre::Color c = fixture.GetColor(data.snapshot, shape_index);
      if (c != theatre::Color::Black()) {
        const auto [r, g, b, max_rgb] = c.GetNormalizedRatios();
        data.cairo->set_source_rgba(r, g, b, 0.5 * max_rgb);
        bool first = true;
        const double tan_beam_angle = std::tan(beam_angle);
        const double cos_tilt = std::cos(tilt);
        const double sin_tilt = std::sin(tilt);

        const double forward_x = cos_tilt * cos_direction;
        const double forward_y = cos_tilt * sin_direction;
        const double forward_z = sin_tilt;

        for (size_t i = 0; i != 80; ++i) {
          // Calculate intersection with floor. Steps:
          // - Start from "top" of the beam, a vector in direction of the x-axis
          // but rotated upward (in y-direction) by the beam angle.
          // - Perform rotation around x-axis by iterator
          // - Unrotate y-axis with tilt degrees
          // - Unrotate z-axis with direction degrees
          // x1 = 1.0;
          // y1 = tan(beam_angle) * cos(iterator);
          // z1 = tan(beam_angle) * sin(iterator);
          // x2 = x1 * cos(tilt) + z1 * sin(tilt);
          // y2 = y1;
          // z2 = z1 * cos(tilt) - x1 * sin(tilt);
          // x3 = y2 * sin(direction) - x2 * cos(direction);
          // y3 = -y2 * cos(direction) - x2 * sin(direction);
          // z3 = z2;
          // Simplified:
          const double iterator = static_cast<double>(i) * (M_PI * 2.0 / 80.0);
          const double sin_iterator = std::sin(iterator);
          const double cos_iterator = std::cos(iterator);
          const double y1 = tan_beam_angle * cos_iterator;
          const double z1 = tan_beam_angle * sin_iterator;
          const double x2 = cos_tilt + z1 * sin_tilt;
          const double x3 = y1 * sin_direction - x2 * cos_direction;
          const double y3 = -y1 * cos_direction - x2 * sin_direction;
          const double z3 = z1 * cos_tilt - sin_tilt;
          if (std::abs(z3) > 1e-3) {
            const double scaling = z / z3;
            const double proj_x = x + x3 * scaling;
            const double proj_y = y + y3 * scaling;
            // Make sure the intersection is in the forward direction
            const double inproduct = x3 * scaling * forward_x +
                                     y3 * scaling * forward_y + z * forward_z;
            if (inproduct > 0.0) {
              if (first) {
                data.cairo->move_to(proj_x, proj_y);
                first = false;
              } else {
                data.cairo->line_to(proj_x, proj_y);
              }
            }
          }
        }
        data.cairo->close_path();
        data.cairo->fill();
      }
      /*
      const double throw_distance =
          std::tan(0.5 * M_PI - tilt) * fixture.GetPosition().Z();
      const double throw_x = x + cos_direction * throw_distance;
      const double throw_y = y + sin_direction * throw_distance;
      data.cairo->set_source_rgb(0.7, 0.2, 0.2);
      data.cairo->move_to(throw_x - 0.5, throw_y - 0.5);
      data.cairo->line_to(throw_x + 0.5, throw_y + 0.5);
      data.cairo->move_to(throw_x - 0.5, throw_y + 0.5);
      data.cairo->line_to(throw_x + 0.5, throw_y - 0.5);
      data.cairo->stroke();
      */
    }
  }
}

void DrawFixtureBeam(const DrawData &data, const theatre::Fixture &fixture) {
  const theatre::FixtureMode &mode = fixture.Mode();
  const theatre::FixtureType &type = mode.Type();
  const size_t shape_count = type.ShapeCount();
  for (size_t shape_index = 0; shape_index != shape_count; ++shape_index) {
    const theatre::Color c = fixture.GetColor(data.snapshot, shape_index);
    if (c != theatre::Color::Black() && type.MinBeamAngle() > 0.0) {
      const double direction =
          fixture.GetBeamDirection(data.snapshot, shape_index);
      const double x = fixture.GetPosition().X() + 0.5;
      const double y = fixture.GetPosition().Y() + 0.5;
      const double z = fixture.GetPosition().Z();
      const double beam_angle =
          type.CanZoom()
              ? mode.GetZoom(fixture, data.snapshot, shape_index) * 0.5
              : type.MinBeamAngle() * 0.5;
      const double tilt = fixture.GetBeamTilt(data.snapshot, shape_index);
      const double cos_tilt = std::cos(tilt);
      const double sin_tilt = std::sin(tilt);
      const double sin_direction = std::sin(direction);
      const double cos_direction = std::cos(direction);
      const double tan_beam_angle = std::tan(beam_angle);
      // Calculate intersection with floor
      const double scaling = sin_tilt < 1e-3 ? 1e3 : z / sin_tilt;
      const double term_1 = tan_beam_angle * sin_direction;
      const double term_2 = cos_tilt * cos_direction;
      const double term_3 = tan_beam_angle * cos_direction;
      const double term_4 = cos_tilt * sin_direction;
      const double direction_1_x = (-term_1 + term_2) * scaling;
      const double direction_1_y = (term_3 + term_4) * scaling;
      const double radius_1 = std::sqrt(direction_1_x * direction_1_x +
                                        direction_1_y * direction_1_y);
      // We only care about the ratio so the scaling is not necessary here
      const double direction_2_x = term_1 + term_2;
      const double direction_2_y = -term_3 + term_4;
      const double direction_1 = std::atan2(direction_1_y, direction_1_x);
      const double direction_2 = std::atan2(direction_2_y, direction_2_x);

      const double radius = GetRadiusFactor(fixture.Symbol().Value()) *
                            data.management.GetTheatre().FixtureSymbolSize();
      const double beam_start_radius = radius * 1.2;
      const double beam_factor = type.MinBeamAngle() / beam_angle;
      const double power_radius =
          radius * (1.2 + type.Brightness() * beam_factor);
      const double beam_end_radius = std::min(power_radius, radius_1);
      Cairo::RefPtr<Cairo::RadialGradient> gradient =
          Cairo::RadialGradient::create(x, y, beam_start_radius, x, y,
                                        beam_end_radius);
      const auto [r, g, b, max_rgb] = c.GetNormalizedRatios();
      gradient->add_color_stop_rgba(0.0, r, g, b, 0.5 * max_rgb);
      gradient->add_color_stop_rgba(1.0, r, g, b, 0.0);
      data.cairo->set_source(gradient);
      const double cos_1 = std::cos(direction_1);
      const double sin_1 = std::sin(direction_1);
      const double cos_2 = std::cos(direction_2);
      const double sin_2 = std::sin(direction_2);
      data.cairo->arc(x, y, beam_start_radius, direction_2, direction_1);
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

void DrawFixture(DrawData &data, const theatre::Fixture &fixture,
                 FixtureState &fixture_state) {
  size_t shapeCount = fixture.Mode().Type().ShapeCount();
  for (size_t i = 0; i != shapeCount; ++i) {
    const size_t shapeIndex = shapeCount - i - 1;
    const theatre::Color c = fixture.GetColor(data.snapshot, shapeIndex);

    data.cairo->set_source_rgb(static_cast<double>(c.Red()) / 224.0 + 0.125,
                               static_cast<double>(c.Green()) / 224.0 + 0.125,
                               static_cast<double>(c.Blue()) / 224.0 + 0.125);

    const double single_radius =
        GetRadiusFactor(fixture.Symbol().Value()) *
        data.management.GetTheatre().FixtureSymbolSize();
    ;
    const double radius =
        shapeCount == 1
            ? single_radius
            : 0.33 + 0.07 * static_cast<double>(shapeIndex) / (shapeCount - 1);
    const double x = fixture.GetPosition().X() + 0.5;
    const double y = fixture.GetPosition().Y() + 0.5;
    data.cairo->arc(x, y, radius, 0.0, 2.0 * M_PI);
    data.cairo->fill();

    // If a fixture is continuously rotating (e.g. a disco ball light), draw a
    // rotating cross.

    const int rotation_speed =
        fixture.GetRotationSpeed(data.snapshot, shapeIndex);
    if (rotation_speed != 0) {
      data.is_moving = true;
      const double displayed_rotation =
          M_PI * static_cast<double>(rotation_speed) *
          data.style.time_since_previous / (10.0 * (1U << 24U));
      fixture_state.continuous_rotation = std::fmod(
          displayed_rotation + fixture_state.continuous_rotation, M_PI);
      const double s = std::sin(fixture_state.continuous_rotation);
      const double c = std::cos(fixture_state.continuous_rotation);
      data.cairo->set_line_width(radius * 0.2);
      data.cairo->set_source_rgb(0, 0, 0);
      data.cairo->move_to(x + c * radius, y + s * radius);
      data.cairo->line_to(x - c * radius, y - s * radius);
      data.cairo->move_to(x + s * radius, y - c * radius);
      data.cairo->line_to(x - s * radius, y + c * radius);
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
    const std::vector<system::ObservingPtr<theatre::Fixture>>
        &selected_fixtures) {
  const std::vector<system::TrackablePtr<theatre::Fixture>> &fixtures =
      management_.GetTheatre().Fixtures();
  cairo->save();
  scale_ = GetScale(management_, style.width, style.height);
  cairo->scale(scale_, scale_);
  x_padding_ = scale_ == 0.0
                   ? 0.0
                   : std::max(0.0, style.width / scale_ -
                                       management_.GetTheatre().Width());
  y_padding_ = scale_ == 0.0
                   ? 0.0
                   : std::max(0.0, style.height / scale_ -
                                       management_.GetTheatre().Depth());
  cairo->translate(x_padding_ * 0.5 + style.x_offset / scale_,
                   y_padding_ * 0.5 + style.y_offset / scale_);

  if (style.draw_borders) {
    cairo->set_source_rgba(0, 0, 0, 1);
    cairo->rectangle(0, 0, management_.GetTheatre().Width(),
                     management_.GetTheatre().Depth());
    cairo->fill_preserve();
    cairo->set_source_rgba(0.5, 0.5, 0.5, 1);
    cairo->set_line_width(1.0 / scale_);
    cairo->stroke();
  }

  DrawData draw_data{cairo, management_, snapshot, style, scale_, false};

  if (style.draw_projections) {
    for (const system::TrackablePtr<theatre::Fixture> &fixture : fixtures) {
      DrawFixtureProjection(draw_data, *fixture);
    }
  }
  if (style.draw_beams) {
    for (const system::TrackablePtr<theatre::Fixture> &fixture : fixtures) {
      if (fixture->IsVisible()) {
        DrawFixtureBeam(draw_data, *fixture);
      }
    }
  }

  if (style.draw_fixtures) {
    state_.resize(fixtures.size());
    for (size_t fixtureIndex = 0; fixtureIndex != fixtures.size();
         ++fixtureIndex) {
      const theatre::Fixture &fixture = *fixtures[fixtureIndex];
      if (fixture.IsVisible()) {
        FixtureState &fixture_state = state_[fixtureIndex];
        DrawFixture(draw_data, fixture, fixture_state);
      }
    }
  }
  is_moving_ = draw_data.is_moving;

  DrawSelectedFixtures(cairo, selected_fixtures);
  cairo->restore();
}

void RenderEngine::DrawSelectedFixtures(
    const Cairo::RefPtr<Cairo::Context> &cairo,
    const std::vector<system::ObservingPtr<theatre::Fixture>>
        &selected_fixtures) const {
  cairo->set_line_width(4.0 / scale_);
  cairo->set_source_rgb(0.2, 0.2, 1.0);
  for (const system::ObservingPtr<theatre::Fixture> &fixture_ptr :
       selected_fixtures) {
    const theatre::Fixture *f = fixture_ptr.Get();
    if (f->IsVisible()) {
      const double direction = f->Direction();
      const double radius = GetRadiusFactor(f->Symbol().Value()) *
                            management_.GetTheatre().FixtureSymbolSize();
      const double x = f->GetPosition().X() + 0.5;
      const double y = f->GetPosition().Y() + 0.5;
      cairo->arc(x, y, radius, 0.0, 2.0 * M_PI);
      const double cos_dir = std::cos(direction);
      const double sin_dir = std::sin(direction);
      const double end_x = x + radius * kRotationHandleEnd * cos_dir;
      const double end_y = y + radius * kRotationHandleEnd * sin_dir;
      cairo->move_to(end_x, end_y);
      cairo->line_to(x + radius * kRotationHandleStart * cos_dir,
                     y + radius * kRotationHandleStart * sin_dir);
      cairo->stroke();
      cairo->arc(end_x, end_y, 5.0 / scale_, 0.0, 2.0 * M_PI);
      cairo->set_source_rgb(0.2, 0.5, 1.0);
      cairo->fill_preserve();
      cairo->set_source_rgb(0.2, 0.2, 1.0);
      cairo->stroke();
    }
  }
}

void RenderEngine::DrawSelectionRectangle(
    const Cairo::RefPtr<Cairo::Context> &cairo,
    const theatre::Coordinate2D &from, const theatre::Coordinate2D &to) const {
  const theatre::Coordinate2D size = to - from;
  cairo->save();
  cairo->scale(scale_, scale_);
  cairo->translate(x_padding_ * 0.5, y_padding_ * 0.5);
  cairo->set_line_width(2.0 / scale_);
  cairo->rectangle(from.X(), from.Y(), size.X(), size.Y());
  cairo->set_source_rgba(0.2, 0.2, 1.0, 0.5);
  cairo->fill_preserve();
  cairo->set_source_rgba(0.5, 0.5, 1.0, 0.8);
  cairo->stroke();
  cairo->restore();
}

system::ObservingPtr<theatre::Fixture> RenderEngine::FixtureAt(
    const theatre::Coordinate2D &position) const {
  const std::vector<system::TrackablePtr<theatre::Fixture>> &fixtures =
      management_.GetTheatre().Fixtures();

  const system::TrackablePtr<theatre::Fixture> *fixture = nullptr;
  double closest = std::numeric_limits<double>::max();
  for (const system::TrackablePtr<theatre::Fixture> &f : fixtures) {
    if (f->IsVisible() &&
        position.InsideRectangle(f->GetXY(), f->GetXY().Add(1.0, 1.0))) {
      const double distanceSq =
          position.SquaredDistance(f->GetXY().Add(0.5, 0.5));
      const double radius = GetRadiusFactor(f->Symbol().Value()) *
                            management_.GetTheatre().FixtureSymbolSize();
      const double radius_squared = radius * radius;
      if (distanceSq <= radius_squared && distanceSq < closest) {
        fixture = &f;
        closest = distanceSq;
      }
    }
  }
  return fixture ? fixture->GetObserver() : nullptr;
}

system::ObservingPtr<theatre::Fixture> RenderEngine::GetDirectionHandleAt(
    const std::vector<system::ObservingPtr<theatre::Fixture>> &fixtures,
    const theatre::Coordinate2D &position) const {
  for (const system::ObservingPtr<theatre::Fixture> &f : fixtures) {
    const double start = 0.5 - kRotationHandleEnd;
    const double end = 0.5 + kRotationHandleEnd;
    if (f->IsVisible() && position.InsideRectangle(f->GetXY().Add(start, start),
                                                   f->GetXY().Add(end, end))) {
      const theatre::Coordinate2D centre = f->GetXY().Add(0.5, 0.5);
      const double distanceSq = position.SquaredDistance(centre);
      const double radius = GetRadiusFactor(f->Symbol().Value()) *
                            management_.GetTheatre().FixtureSymbolSize();
      const double kStartSquared =
          kRotationHandleStart * kRotationHandleStart * radius * radius;
      const double kEndSquared =
          kRotationHandleEnd * kRotationHandleEnd * radius * radius;
      if (distanceSq >= kStartSquared && distanceSq < kEndSquared) {
        const double direction = f->Direction();
        const double cos_dir = std::cos(direction);
        const double sin_dir = std::sin(direction);
        double mouse_direction_diff =
            std::fmod((position - centre).Angle() - direction, 2.0 * M_PI);
        if (mouse_direction_diff < -M_PI)
          mouse_direction_diff += 2.0 * M_PI;
        else if (mouse_direction_diff > M_PI)
          mouse_direction_diff -= 2.0 * M_PI;
        const bool same_side = mouse_direction_diff < 0.5 * M_PI &&
                               mouse_direction_diff > -0.5 * M_PI;
        if (same_side &&
            std::abs(system::DistanceToLine(
                position.X(), position.Y(), centre.X(), centre.Y(),
                centre.X() + cos_dir, centre.Y() + sin_dir)) < 0.15) {
          return f;
        }
      }
    }
  }
  return nullptr;
}

theatre::Coordinate2D RenderEngine::MouseToPosition(double mouse_x,
                                                    double mouse_y,
                                                    double width,
                                                    double height) const {
  const double scale = GetScale(management_, width, height);
  if (scale == 0.0) return theatre::Coordinate2D(0.0, 0.0);
  const theatre::Coordinate2D padding(
      std::max(0.0 / scale, width / scale - management_.GetTheatre().Width()),
      std::max(0.0 / scale, height / scale - management_.GetTheatre().Depth()));
  return theatre::Coordinate2D(mouse_x, mouse_y) / scale - padding * 0.5;
}

}  // namespace glight::gui
