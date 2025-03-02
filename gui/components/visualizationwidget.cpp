#include "visualizationwidget.h"

#include "gui/eventtransmitter.h"
#include "gui/functions.h"
#include "gui/instance.h"

#include "gui/windows/addfixturewindow.h"
#include "gui/windows/designwizard.h"
#include "gui/windows/fixtureproperties.h"

#include "gui/mainwindow/actions.h"
#include "gui/mainwindow/mainwindow.h"

#include "system/math.h"
#include "system/trackableptr.h"
#include "system/midi/manager.h"

#include "theatre/fixture.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturegroup.h"
#include "theatre/management.h"
#include "theatre/managementtools.h"
#include "theatre/theatre.h"

#include "theatre/devices/beatfinder.h"

#include <glibmm/main.h>

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/main.h>

#include <cmath>
#include <iostream>  //DEBUG
#include <memory>
#include <optional>

namespace glight::gui {

using system::ObservingPtr;

namespace {
theatre::Coordinate2D RotateFixtures(
    std::vector<ObservingPtr<theatre::Fixture>> &fixture_list,
    ObservingPtr<theatre::Fixture> &centre_of_rotation,
    theatre::Coordinate2D from, theatre::Coordinate2D to) {
  const theatre::Coordinate2D centre =
      centre_of_rotation->GetXY() + theatre::Coordinate2D(0.5, 0.5);
  const double from_angle = (from - centre).Angle();
  double rotation = (to - centre).Angle() - from_angle;
  rotation = std::round(rotation / (M_PI * 0.125)) * M_PI * 0.125;
  if (rotation == 0.0) {
    return from;
  } else {
    for (const ObservingPtr<theatre::Fixture> &fixture : fixture_list) {
      const double new_direction =
          std::fmod(fixture->Direction() + rotation, 2.0 * M_PI);
      fixture->SetDirection(new_direction < 0.0 ? new_direction + 2.0 * M_PI
                                                : new_direction);
    }
    return centre + theatre::Coordinate2D(std::cos(from_angle + rotation),
                                          std::sin(from_angle + rotation));
  }
}
}  // namespace

VisualizationWidget::VisualizationWidget(theatre::Management *management,
                                         EventTransmitter *eventTransmitter,
                                         FixtureSelection *fixtureSelection,
                                         MainWindow *showWindow)
    : _management(management),
      _eventTransmitter(eventTransmitter),
      _globalSelection(fixtureSelection),
      main_window_(showWindow),
      _isInitialized(false),
      _isTimerRunning(false),
      _dragType(MouseState::Normal),

      render_engine_(*management) {
  set_size_request(64, 64);

  _globalSelectionConnection = _globalSelection->SignalChange().connect(
      [&]() { onGlobalSelectionChanged(); });

  set_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
             Gdk::POINTER_MOTION_MASK);
  signal_draw().connect(sigc::mem_fun(*this, &VisualizationWidget::onExpose));
  signal_button_press_event().connect(
      sigc::mem_fun(*this, &VisualizationWidget::onButtonPress));
  signal_button_release_event().connect(
      sigc::mem_fun(*this, &VisualizationWidget::onButtonRelease));
  signal_motion_notify_event().connect(
      sigc::mem_fun(*this, &VisualizationWidget::onMotion));
  inializeContextMenu();
  primary_snapshot_ = _management->PrimarySnapshot();
  secondary_snapshot_ = _management->SecondarySnapshot();
  update_connection_ = _eventTransmitter->SignalUpdateControllables().connect(
      [&]() { Update(); });
}

void VisualizationWidget::inializeContextMenu() {
  std::vector<theatre::FixtureSymbol::Symbol> symbols(
      theatre::FixtureSymbol::List());

  mi_set_full_on_.signal_activate().connect([&]() {
    theatre::SetAllFixtures(*_management, _selectedFixtures, Color::White());
  });
  set_menu_.add(mi_set_full_on_);
  mi_set_off_.signal_activate().connect([&]() {
    theatre::SetAllFixtures(*_management, _selectedFixtures, Color::Black());
  });
  set_menu_.add(mi_set_off_);
  mi_set_color_.signal_activate().connect([&]() { OnSetColor(); });
  set_menu_.add(mi_set_color_);

  mi_track_pan_.signal_activate().connect([&]() { OnTrackWithPan(); });
  set_menu_.add(mi_track_pan_);

  mi_set_menu_.set_submenu(set_menu_);
  _popupMenu.add(mi_set_menu_);

  _miSymbols.reserve(symbols.size());
  for (theatre::FixtureSymbol::Symbol symbol : symbols) {
    _miSymbols.emplace_back(theatre::FixtureSymbol(symbol).Name());
    _miSymbols.back().signal_activate().connect(
        [this, symbol]() { onSetSymbol(symbol); });
    _symbolMenu.add(_miSymbols.back());
  }

  _miSymbolMenu.set_submenu(_symbolMenu);
  _popupMenu.add(_miSymbolMenu);

  Gtk::RadioMenuItem::Group dryModeStyleGroup;
  _miDMSPrimary.set_group(dryModeStyleGroup);
  _miDMSPrimary.set_active(true);
  _miDMSPrimary.signal_activate().connect([&]() { Update(); });
  _dryModeStyleMenu.add(_miDMSPrimary);
  _miDMSSecondary.set_group(dryModeStyleGroup);
  _miDMSSecondary.signal_activate().connect([&]() { Update(); });
  _dryModeStyleMenu.add(_miDMSSecondary);
  _miDMSHorizontal.set_group(dryModeStyleGroup);
  _miDMSHorizontal.signal_activate().connect([&]() { Update(); });
  _dryModeStyleMenu.add(_miDMSHorizontal);
  _miDMSVertical.set_group(dryModeStyleGroup);
  _miDMSVertical.signal_activate().connect([&]() { Update(); });
  _dryModeStyleMenu.add(_miDMSVertical);
  _miDMSShadow.set_group(dryModeStyleGroup);
  _miDMSShadow.signal_activate().connect([&]() { Update(); });
  _dryModeStyleMenu.add(_miDMSShadow);

  _miDryModeStyle.set_submenu(_dryModeStyleMenu);
  _popupMenu.add(_miDryModeStyle);

  _miAlignHorizontally.signal_activate().connect(
      [&] { onAlignHorizontally(); });
  _popupMenu.add(_miAlignHorizontally);

  _miAlignVertically.signal_activate().connect([&] { onAlignVertically(); });
  _popupMenu.add(_miAlignVertically);

  _miDistributeEvenly.signal_activate().connect([&] { onDistributeEvenly(); });
  _popupMenu.add(_miDistributeEvenly);

  _popupMenu.add(_miSeparator1);

  _miAdd.signal_activate().connect([&] { onAddFixtures(); });
  _popupMenu.add(_miAdd);

  _miAddPreset.signal_activate().connect([&] { onAddPreset(); });
  _popupMenu.add(_miAddPreset);

  _miRemove.signal_activate().connect([&] { onRemoveFixtures(); });
  _popupMenu.add(_miRemove);

  _miGroup.signal_activate().connect([&] { onGroupFixtures(); });
  _popupMenu.add(_miGroup);

  _miDesign.signal_activate().connect([&] { onDesignFixtures(); });
  _popupMenu.add(_miDesign);

  _popupMenu.add(_miSeparator2);

  _miProperties.signal_activate().connect([&] { onFixtureProperties(); });
  _popupMenu.add(_miProperties);

  _miSaveImage.signal_activate().connect([&] { onSaveImage(); });
  _popupMenu.add(_miSaveImage);

  _popupMenu.show_all_children();
}

void VisualizationWidget::initialize() {
  queue_draw();
  _isInitialized = true;

  if (!_isTimerRunning) {
    _timeoutConnection = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &VisualizationWidget::onTimeout), 40);
    _isTimerRunning = true;
  }
}

void VisualizationWidget::onTheatreChanged() {
  for (size_t i = _selectedFixtures.size(); i != 0; --i) {
    if (!_management->GetTheatre().Contains(*_selectedFixtures[i - 1]))
      _selectedFixtures.erase(_selectedFixtures.begin() + i - 1);
  }
  Update();
}

bool VisualizationWidget::onExpose(
    const Cairo::RefPtr<Cairo::Context> &context) {
  if (!_isInitialized) initialize();

  drawAll(context);
  return true;
}

void VisualizationWidget::updateMidiColors() {
  const std::vector<system::TrackablePtr<theatre::Fixture>> &fixtures =
      _management->GetTheatre().Fixtures();
  system::midi::Manager &midi_manager = main_window_->GetMidiManager();
  const size_t n_pads = midi_manager.GetNPads();
  if (n_pads > 0) {
    size_t pad = 0;
    for (const system::TrackablePtr<theatre::Fixture> &fixture : fixtures) {
      if (fixture->IsVisible()) {
        const glight::theatre::FixtureType &type = fixture->Type();
        const size_t shape_count = type.ShapeCount();
        for (size_t shape_index = 0; shape_index != shape_count;
             ++shape_index) {
          const theatre::Color color =
              fixture->GetColor(primary_snapshot_, shape_index);
          midi_manager.SetFixtureColor(pad % 8, pad / 8, color, false);
          ++pad;
          if (pad >= n_pads) return;
        }
      }
    }

    double beat;
    double confidence;
    _management->GetBeatFinder().GetBeatValue(beat, confidence);
    if (confidence > 0.0)
      midi_manager.SetBeat(system::OptionalNumber<double>(beat));
    else
      midi_manager.SetBeat(system::OptionalNumber<double>());
  }
}

DryModeStyle VisualizationWidget::GetDryModeStyle() const {
  if (_miDMSSecondary.get_active())
    return DryModeStyle::Secondary;
  else if (_miDMSHorizontal.get_active())
    return DryModeStyle::Horizontal;
  else if (_miDMSVertical.get_active())
    return DryModeStyle::Vertical;
  else if (_miDMSShadow.get_active())
    return DryModeStyle::Shadow;
  else
    return DryModeStyle::Primary;
}

struct DrawInfo {
  size_t width;
  size_t height;
  size_t x_offset;
  size_t y_offset;

  void AssignTo(DrawStyle &style) const {
    style.width = width;
    style.height = height;
    style.x_offset = x_offset;
    style.y_offset = y_offset;
  }
};

std::optional<DrawInfo> GetPrimaryStyleDimensions(DryModeStyle style,
                                                  size_t width, size_t height) {
  std::optional<DrawInfo> draw_info;
  switch (style) {
    case DryModeStyle::Primary:
      draw_info.emplace();
      draw_info->x_offset = 0;
      draw_info->y_offset = 0;
      draw_info->width = width;
      draw_info->height = height;
      break;
    case DryModeStyle::Secondary:
      break;
    case DryModeStyle::Horizontal:
      draw_info.emplace();
      draw_info->y_offset = 0.0;
      draw_info->x_offset = 0.0;
      draw_info->width = width / 2;
      draw_info->height = height;
      break;
    case DryModeStyle::Vertical:
      draw_info.emplace();
      draw_info->x_offset = 0.0;
      draw_info->y_offset = 0.0;
      draw_info->width = width;
      draw_info->height = height / 2;
      break;
    case DryModeStyle::Shadow:
      draw_info.emplace();
      draw_info->x_offset = width * 1 / 100;
      draw_info->y_offset = height * 1 / 100;
      draw_info->width = width * 99 / 100;
      draw_info->height = height * 99 / 100;
      break;
  }
  return draw_info;
}

std::optional<DrawInfo> GetSecondaryStyleDimensions(DryModeStyle style,
                                                    size_t width,
                                                    size_t height) {
  std::optional<DrawInfo> draw_info;
  switch (style) {
    case DryModeStyle::Primary:
      break;
    case DryModeStyle::Secondary:
    case DryModeStyle::Shadow:
      draw_info.emplace();
      draw_info->x_offset = 0;
      draw_info->y_offset = 0;
      draw_info->width = width;
      draw_info->height = height;
      break;
    case DryModeStyle::Horizontal:
      draw_info.emplace();
      draw_info->y_offset = 0.0;
      draw_info->x_offset = width / 2;
      draw_info->width = width - draw_info->x_offset;
      draw_info->height = height;
      break;
    case DryModeStyle::Vertical:
      draw_info.emplace();
      draw_info->x_offset = 0.0;
      draw_info->y_offset = height / 2;
      draw_info->width = width;
      draw_info->height = height - draw_info->y_offset;
      break;
  }
  return draw_info;
}

void VisualizationWidget::DrawShapshot(
    const Cairo::RefPtr<Cairo::Context> &cairo,
    const std::vector<system::ObservingPtr<theatre::Fixture>> &selection,
    size_t width, size_t height) {
  updateMidiColors();

  cairo->set_source_rgba(0.1, 0.1, 0.2, 1);
  cairo->rectangle(0, 0, width, height);
  cairo->fill();

  const double time = _management->GetOffsetTimeInMS();
  static double previous_time = time;

  DrawStyle style;
  style.time_since_previous = time - previous_time;
  style.draw_background = true;
  style.draw_fixtures = draw_fixtures_;
  style.draw_beams = draw_beams_;
  style.draw_projections = draw_projections_;
  style.draw_borders = draw_borders_;
  previous_time = time;
  const DryModeStyle dry_mode = GetDryModeStyle();
  if (const std::optional<DrawInfo> draw_info =
          GetPrimaryStyleDimensions(dry_mode, width, height);
      draw_info) {
    draw_info->AssignTo(style);
    render_engine_.DrawSnapshot(cairo, primary_snapshot_, style, selection);
  }
  if (const std::optional<DrawInfo> draw_info =
          GetSecondaryStyleDimensions(dry_mode, width, height);
      draw_info) {
    draw_info->AssignTo(style);
    render_engine_.DrawSnapshot(cairo, secondary_snapshot_, style, selection);
  }
}

void VisualizationWidget::drawAll(const Cairo::RefPtr<Cairo::Context> &cairo) {
  const size_t width = get_width();
  const size_t height = get_height();

  DrawShapshot(cairo, _selectedFixtures, width, height);

  if (_dragType == MouseState::DragRectangle ||
      _dragType == MouseState::DragAddRectangle) {
    render_engine_.DrawSelectionRectangle(cairo, _draggingStart, _draggingTo);
  }
}

bool VisualizationWidget::onButtonPress(GdkEventButton *event) {
  if (event->button == 1) {
    const bool shift =
        (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) ==
        GDK_SHIFT_MASK;
    const std::optional<DrawInfo> info =
        GetPrimaryStyleDimensions(GetDryModeStyle(), get_width(), get_height());
    if (info) {
      const theatre::Coordinate2D pos = render_engine_.MouseToPosition(
          event->x, event->y, info->width, info->height);
      system::ObservingPtr<theatre::Fixture> clicked_fixture =
          render_engine_.FixtureAt(pos);
      if (shift) {
        if (clicked_fixture) {
          // Was a fixture clicked that was already selected? Then unselect.
          // If not, add the clicked fixture to the selection
          auto iterator = std::find(_selectedFixtures.begin(),
                                    _selectedFixtures.end(), clicked_fixture);
          if (iterator == _selectedFixtures.end()) {
            _selectedFixtures.emplace_back(clicked_fixture);
            _globalSelection->SetSelection(_selectedFixtures);
          } else {
            _selectedFixtures.erase(iterator);
            _globalSelection->SetSelection(_selectedFixtures);
          }
        }
      } else {
        if (clicked_fixture) {
          // Was a fixture clicked that was already selected? Then keep all
          // selected. If not, select the clicked fixture
          if (std::find(_selectedFixtures.begin(), _selectedFixtures.end(),
                        clicked_fixture) == _selectedFixtures.end()) {
            _selectedFixtures.assign(1, clicked_fixture);
            _globalSelection->SetSelection(_selectedFixtures);
          }
        } else {
          system::ObservingPtr<theatre::Fixture> clicked_handle =
              render_engine_.GetDirectionHandleAt(_selectedFixtures, pos);
          if (clicked_handle) {
            _dragType = MouseState::RotateFixture;
            _dragInvolvedFixtures = {clicked_handle};
          } else {
            _selectedFixtures.clear();
            _globalSelection->SetSelection(_selectedFixtures);
          }
        }
      }

      _draggingStart = pos;
      if (shift) {
        _dragType = MouseState::DragAddRectangle;
        _draggingTo = pos;
        _dragInvolvedFixtures = _selectedFixtures;
      } else if (_dragType != MouseState::RotateFixture) {
        if (!_selectedFixtures.empty()) {
          _dragType = MouseState::DragFixture;
        } else {
          _dragType = MouseState::DragRectangle;
          _draggingTo = pos;
        }
      }
      queue_draw();
    }
  }
  if (event->button == 3) {
    queue_draw();
    const bool enable = !Instance::State().LayoutLocked();
    const bool has_selection = !_selectedFixtures.empty();
    const bool selection_enabled = enable && has_selection;
    const bool dual_enabled = enable && _selectedFixtures.size() >= 2;
    mi_set_menu_.set_sensitive(has_selection);
    _miAlignHorizontally.set_sensitive(dual_enabled);
    _miAlignVertically.set_sensitive(dual_enabled);
    _miDistributeEvenly.set_sensitive(dual_enabled);
    _miAdd.set_sensitive(enable);
    _miAddPreset.set_sensitive(enable);
    _miRemove.set_sensitive(selection_enabled);
    _miSymbolMenu.set_sensitive(selection_enabled);
    _miProperties.set_sensitive(selection_enabled);
    _popupMenu.popup_at_pointer(reinterpret_cast<GdkEvent *>(event));
  }
  return true;
}

bool VisualizationWidget::onButtonRelease(GdkEventButton *event) {
  if (event->button == 1) {
    if (_dragType == MouseState::DragFixture) {
      const std::optional<DrawInfo> info = GetPrimaryStyleDimensions(
          GetDryModeStyle(), get_width(), get_height());
      if (info) {
        _draggingStart = render_engine_.MouseToPosition(
            event->x, event->y, info->width, info->height);
      }
    }
    if (_dragType == MouseState::TrackPan ||
        _dragType == MouseState::RotateFixture) {
      _dragType = MouseState::Normal;
    } else {
      _globalSelection->SetSelection(_selectedFixtures);
      _dragType = MouseState::Normal;
      _dragInvolvedFixtures.clear();
      queue_draw();
    }
  }
  return true;
}

void VisualizationWidget::SetCursor(Gdk::CursorType cursor_type) {
  if (cursor_type_ != cursor_type) {
    cursor_type_ = cursor_type;
    get_window()->set_cursor(Gdk::Cursor::create(get_display(), cursor_type));
  }
}

bool VisualizationWidget::onMotion(GdkEventMotion *event) {
  const std::optional<DrawInfo> info =
      GetPrimaryStyleDimensions(GetDryModeStyle(), get_width(), get_height());
  if (info) {
    const theatre::Coordinate2D pos = render_engine_.MouseToPosition(
        event->x, event->y, info->width, info->height);
    switch (_dragType) {
      case MouseState::Normal: {
        Gdk::CursorType cursor = Gdk::CursorType::ARROW;
        system::ObservingPtr<theatre::Fixture> hover_fixture =
            render_engine_.FixtureAt(pos);
        if (!hover_fixture) {
          system::ObservingPtr<theatre::Fixture> hover_handle =
              render_engine_.GetDirectionHandleAt(_selectedFixtures, pos);
          if (hover_handle) {
            cursor = Gdk::CursorType::CROSS;
          }
        }
        SetCursor(cursor);
      } break;
      case MouseState::DragFixture:
        if (!Instance::State().LayoutLocked()) {
          for (const system::ObservingPtr<theatre::Fixture> &fixture :
               _selectedFixtures)
            fixture->SetXY(fixture->GetXY() + pos - _draggingStart);
          _draggingStart = pos;
        }
        break;
      case MouseState::DragRectangle:
        _draggingTo = pos;
        selectFixtures(_draggingStart, _draggingTo);
        break;
      case MouseState::DragAddRectangle:
        _draggingTo = pos;
        addFixtures(_draggingStart, _draggingTo);
        break;
      case MouseState::TrackPan:
        SetPan(pos);
        break;
      case MouseState::RotateFixture:
        _draggingStart =
            RotateFixtures(_selectedFixtures, _dragInvolvedFixtures.front(),
                           _draggingStart, pos);
        break;
    }
    queue_draw();
  }
  return true;
}

void VisualizationWidget::selectFixtures(const theatre::Coordinate2D &a,
                                         const theatre::Coordinate2D &b) {
  _selectedFixtures.clear();
  double x1 = a.X();
  double y1 = a.Y();
  double x2 = b.X();
  double y2 = b.Y();
  if (x1 > x2) std::swap(x1, x2);
  if (y1 > y2) std::swap(y1, y2);
  theatre::Coordinate2D first(x1 - 0.1, y1 - 0.1);
  theatre::Coordinate2D second(x2 - 0.9, y2 - 0.9);
  if (second.X() - first.X() > 0.0 && second.Y() - first.Y() > 0.0) {
    const std::vector<system::TrackablePtr<theatre::Fixture>> &fixtures =
        _management->GetTheatre().Fixtures();
    for (const system::TrackablePtr<theatre::Fixture> &fixture : fixtures) {
      if (fixture->IsVisible() &&
          fixture->GetXY().InsideRectangle(first, second))
        _selectedFixtures.emplace_back(fixture.GetObserver());
    }
  }
}

void VisualizationWidget::addFixtures(const theatre::Coordinate2D &a,
                                      const theatre::Coordinate2D &b) {
  selectFixtures(a, b);
  for (const system::ObservingPtr<theatre::Fixture> &fixture :
       _dragInvolvedFixtures) {
    auto iter =
        std::find(_selectedFixtures.begin(), _selectedFixtures.end(), fixture);
    if (iter == _selectedFixtures.end())
      _selectedFixtures.emplace_back(fixture);
    else
      _selectedFixtures.erase(iter);
  }
}

void VisualizationWidget::onAlignHorizontally() {
  if (_selectedFixtures.size() >= 2) {
    double y = 0.0;

    for (const system::ObservingPtr<theatre::Fixture> &fixture :
         _selectedFixtures)
      y += fixture->GetPosition().Y();

    y /= _selectedFixtures.size();

    for (const system::ObservingPtr<theatre::Fixture> &fixture :
         _selectedFixtures)
      fixture->GetPosition().Y() = y;
  }
}

void VisualizationWidget::onAlignVertically() {
  if (_selectedFixtures.size() >= 2) {
    double x = 0.0;

    for (const system::ObservingPtr<theatre::Fixture> &fixture :
         _selectedFixtures)
      x += fixture->GetPosition().X();

    x /= _selectedFixtures.size();

    for (const system::ObservingPtr<theatre::Fixture> &fixture :
         _selectedFixtures)
      fixture->GetPosition().X() = x;
  }
}

void VisualizationWidget::onDistributeEvenly() {
  if (_selectedFixtures.size() >= 2) {
    double left = _selectedFixtures[0]->GetPosition().X();
    double right = _selectedFixtures[0]->GetPosition().X();
    double top = _selectedFixtures[0]->GetPosition().Y();
    double bottom = _selectedFixtures[0]->GetPosition().Y();
    double highest = _selectedFixtures[0]->GetPosition().Z();
    double lowest = _selectedFixtures[0]->GetPosition().Z();

    for (size_t i = 1; i != _selectedFixtures.size(); ++i) {
      const theatre::Fixture *fixture = _selectedFixtures[i].Get();
      left = std::min(fixture->GetPosition().X(), left);
      right = std::max(fixture->GetPosition().X(), right);
      top = std::min(fixture->GetPosition().Y(), top);
      bottom = std::max(fixture->GetPosition().Y(), bottom);
      highest = std::min(fixture->GetPosition().Z(), highest);
      lowest = std::max(fixture->GetPosition().Z(), lowest);
    }

    std::vector<theatre::Fixture *> list;
    list.reserve(_selectedFixtures.size());
    for (const system::ObservingPtr<theatre::Fixture> &fixture :
         _selectedFixtures)
      list.emplace_back(fixture.Get());
    if (left == right) {
      if (top == bottom) {
        std::sort(list.begin(), list.end(),
                  [](const theatre::Fixture *a, const theatre::Fixture *b) {
                    return a->GetPosition().Z() < b->GetPosition().Z();
                  });
        for (size_t i = 0; i != list.size(); ++i) {
          const double z = static_cast<double>(i) /
                               static_cast<double>(list.size() - 1) *
                               (bottom - top) +
                           top;
          list[i]->GetPosition().Z() = z;
        }
      } else {
        std::sort(list.begin(), list.end(),
                  [](const theatre::Fixture *a, const theatre::Fixture *b) {
                    return a->GetPosition().Y() < b->GetPosition().Y();
                  });
        for (size_t i = 0; i != list.size(); ++i) {
          const double y = static_cast<double>(i) /
                               static_cast<double>(list.size() - 1) *
                               (bottom - top) +
                           top;
          const double z = static_cast<double>(i) /
                               static_cast<double>(list.size() - 1) *
                               (highest - lowest) +
                           highest;
          list[i]->GetPosition() = {left, y, z};
        }
      }
    } else {
      std::sort(list.begin(), list.end(),
                [](const theatre::Fixture *a, const theatre::Fixture *b) {
                  return a->GetPosition().X() < b->GetPosition().X();
                });
      left = list.front()->GetPosition().X();
      right = list.back()->GetPosition().X();
      top = list.front()->GetPosition().Y();
      bottom = list.back()->GetPosition().Y();
      for (size_t i = 0; i != list.size(); ++i) {
        double r =
            static_cast<double>(i) / static_cast<double>(list.size() - 1);
        const double x = r * (right - left) + left;
        const double y = r * (bottom - top) + top;
        const double z = r * (highest - lowest) + highest;
        list[i]->GetPosition() = {x, y, z};
      }
    }
  }
}

void VisualizationWidget::onAddFixtures() {
  sub_window_ = std::make_unique<windows::AddFixtureWindow>(_eventTransmitter,
                                                            *_management);
  sub_window_->set_transient_for(*main_window_);
  sub_window_->set_modal(true);
  sub_window_->show();
}

void VisualizationWidget::onAddPreset() {
  const std::set<system::ObservingPtr<theatre::Fixture>, std::less<>>
      fixture_set(_selectedFixtures.begin(), _selectedFixtures.end());
  theatre::Folder &folder = main_window_->SelectedFolder();
  mainwindow::NewPresetFromFixtures(folder, fixture_set);
}

void VisualizationWidget::onRemoveFixtures() {
  for (const system::ObservingPtr<theatre::Fixture> &fixture :
       _selectedFixtures) {
    std::lock_guard<std::mutex> lock(_management->Mutex());
    _management->RemoveFixture(*fixture);
  }
  _selectedFixtures.clear();
  Instance::Selection().UpdateAfterDelete();
  _eventTransmitter->EmitUpdate();
}

void VisualizationWidget::onGroupFixtures() {
  std::unique_lock lock(_management->Mutex());
  theatre::Folder &parent = main_window_->SelectedFolder();
  const std::string name = parent.GetAvailableName("group");
  theatre::FixtureGroup &group = *_management->AddFixtureGroup(parent, name);
  for (const system::ObservingPtr<theatre::Fixture> &fixture :
       _selectedFixtures) {
    group.Insert(fixture);
  }
  lock.unlock();
  _eventTransmitter->EmitUpdate();
  main_window_->OpenPropertiesWindow(group);
}

void VisualizationWidget::onDesignFixtures() {
  std::unique_ptr<DesignWizard> &designWizard = main_window_->GetDesignWizard();
  designWizard = std::make_unique<DesignWizard>();
  designWizard->SetCurrentPath(main_window_->SelectedFolder().FullPath());
  designWizard->Select(_selectedFixtures);
  designWizard->present();
}

void VisualizationWidget::onFixtureProperties() {
  if (!_propertiesWindow) {
    _propertiesWindow = system::MakeDeletable<windows::FixtureProperties>();
  }
  _propertiesWindow->present();
}

void VisualizationWidget::onSaveImage() {
  Gtk::FileChooserDialog dialog("Save image", Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Save", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("Scalable Vector Graphics (*.svg)");
  filter->add_pattern("*.svg");
  filter->add_mime_type("image/svg+xml");
  dialog.add_filter(filter);

  const int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    Glib::ustring filename(dialog.get_filename());
    if (filename.find('.') == Glib::ustring::npos) filename += ".svg";

    const size_t width = get_width();
    const size_t height = get_height();
    const Cairo::RefPtr<Cairo::SvgSurface> surface =
        Cairo::SvgSurface::create(filename, width, height);
    const Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create(surface);
    DrawShapshot(cairo, {}, width, height);
    cairo->show_page();
    surface->finish();
  }
}

void VisualizationWidget::onSetSymbol(theatre::FixtureSymbol::Symbol symbol) {
  for (const system::ObservingPtr<theatre::Fixture> &fixture :
       _selectedFixtures) {
    fixture->SetSymbol(theatre::FixtureSymbol(symbol));
  }
  if (symbol == theatre::FixtureSymbol::Hidden) _selectedFixtures.clear();
  _eventTransmitter->EmitUpdate();
}

void VisualizationWidget::onGlobalSelectionChanged() {
  _selectedFixtures = _globalSelection->Selection();
  queue_draw();
}

void VisualizationWidget::OnSetColor() {
  std::optional<theatre::Color> color =
      OpenColorDialog(*main_window_, theatre::Color::RedC());
  if (color) {
    theatre::SetAllFixtures(*_management, _selectedFixtures, *color);
  }
}

bool VisualizationWidget::onTimeout() {
  const glight::theatre::ValueSnapshot primary_snapshot =
      _management->PrimarySnapshot();
  const glight::theatre::ValueSnapshot secondary_snapshot =
      _management->SecondarySnapshot();
  if (render_engine_.IsMoving() || primary_snapshot_ != primary_snapshot ||
      secondary_snapshot_ != secondary_snapshot) {
    primary_snapshot_ = primary_snapshot;
    secondary_snapshot_ = secondary_snapshot;
    Update();
  }
  return true;
}

void VisualizationWidget::OnTrackWithPan() { _dragType = MouseState::TrackPan; }

void VisualizationWidget::SetPan(const theatre::Coordinate2D &position) {
  theatre::Management &management = Instance::Management();
  bool is_changed = false;
  for (const system::ObservingPtr<theatre::Fixture> &fixture :
       _selectedFixtures) {
    if (fixture->Type().CanBeamRotate()) {
      constexpr theatre::Coordinate2D offset(0.5, 0.5);
      const theatre::Coordinate2D direction =
          position - fixture->GetXY() - offset;
      const bool is_zero = direction.Y() == 0.0 && direction.X() == 0.0;
      const double angle =
          is_zero ? 0.0 : std::atan2(direction.Y(), direction.X());
      double begin_pan = fixture->Type().MinPan();
      double end_pan = fixture->Type().MaxPan();
      if (fixture->IsUpsideDown()) {
        std::swap(begin_pan, end_pan);
      }
      const double min_value = std::min(begin_pan, end_pan);
      const double max_value = std::max(begin_pan, end_pan);
      const double d_angle = system::RadialClamp(angle - fixture->Direction(),
                                                 min_value, max_value);
      const double scaling = (d_angle - begin_pan) / (end_pan - begin_pan);
      theatre::FixtureControl &control =
          *management.GetFixtureControl(*fixture);
      for (size_t i = 0; i != control.NInputs(); ++i) {
        if (control.InputType(i) == theatre::FunctionType::Pan) {
          theatre::SourceValue *source = management.GetSourceValue(control, i);
          source->A().Set(theatre::ControlValue::FromRatio(scaling).UInt());
          is_changed = true;
        }
      }
    }
  }
  if (is_changed) Update();
}

}  // namespace glight::gui
