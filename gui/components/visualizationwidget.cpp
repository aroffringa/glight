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
#include "theatre/fixturetype.h"
#include "theatre/management.h"
#include "theatre/managementtools.h"
#include "theatre/theatre.h"

#include "theatre/devices/beatfinder.h"

#include <glibmm/main.h>

#include <gtkmm/eventcontrollermotion.h>
#include <gtkmm/filechooserdialog.h>

#include <sigc++/signal.h>

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

      render_engine_(*management),
      context_menu_(*showWindow) {
  set_size_request(64, 64);

  _globalSelectionConnection = _globalSelection->SignalChange().connect(
      [&]() { onGlobalSelectionChanged(); });

  set_draw_func([&](const Cairo::RefPtr<Cairo::Context> &cairo, int, int) {
    VisualizationWidget::onExpose(cairo);
  });

  left_gesture_ = Gtk::GestureClick::create();
  left_gesture_->set_button(1);
  left_gesture_->signal_pressed().connect(
      sigc::mem_fun(*this, &VisualizationWidget::onLeftButtonPress));
  left_gesture_->signal_released().connect(
      sigc::mem_fun(*this, &VisualizationWidget::onLeftButtonRelease));
  add_controller(left_gesture_);

  auto right_gesture = Gtk::GestureClick::create();
  right_gesture->set_button(3);
  right_gesture->signal_pressed().connect(
      sigc::mem_fun(*this, &VisualizationWidget::onRightButtonPress));
  add_controller(right_gesture);

  auto motion = Gtk::EventControllerMotion::create();
  motion->signal_motion().connect(
      sigc::mem_fun(*this, &VisualizationWidget::onMotion));
  add_controller(motion);

  initializeContextMenu();
  primary_snapshot_ = _management->PrimarySnapshot();
  secondary_snapshot_ = _management->SecondarySnapshot();
  update_connection_ = _eventTransmitter->SignalUpdateControllables().connect(
      [&]() { Update(); });
}

VisualizationWidget::~VisualizationWidget() { context_menu_.unparent(); }

void VisualizationWidget::initializeContextMenu() {
  context_menu_.SignalSetFullOn.connect([&]() {
    theatre::SetAllFixtures(*_management, _selectedFixtures, Color::White());
  });
  context_menu_.SignalSetOff.connect([&]() {
    theatre::SetAllFixtures(*_management, _selectedFixtures, Color::Black());
  });
  context_menu_.SignalSetColor.connect([&]() { OnSetColor(); });
  context_menu_.SignalTrack.connect([&]() { OnTrack(); });
  context_menu_.SignalTrackPan.connect([&]() { OnTrackWithPan(); });
  context_menu_.SignalSelectSymbol.connect(
      [&](theatre::FixtureSymbol::Symbol symbol) { onSetSymbol(symbol); });
  context_menu_.SignalDryStyleChange.connect([&]() { Update(); });
  context_menu_.SignalAlignHorizontally.connect(
      [&]() { onAlignHorizontally(); });
  context_menu_.SignalAlignVertically.connect([&]() { onAlignVertically(); });
  context_menu_.SignalDistributeEvenly.connect([&]() { onDistributeEvenly(); });
  context_menu_.SignalAddFixtures.connect([&]() { onAddFixtures(); });
  context_menu_.SignalAddPreset.connect([&]() { onAddPreset(); });
  context_menu_.SignalRemoveFixtures.connect([&]() { onRemoveFixtures(); });
  context_menu_.SignalGroupFixtures.connect([&]() { onGroupFixtures(); });
  context_menu_.SignalDesignFixtures.connect([&]() { onDesignFixtures(); });
  context_menu_.SignalFixtureProperties.connect(
      [&]() { onFixtureProperties(); });
  context_menu_.SignalSaveImage.connect([&]() { onSaveImage(); });
  context_menu_.set_parent(*this);
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
  const auto iter =
      std::remove(_selectedFixtures.begin(), _selectedFixtures.end(), nullptr);
  if (iter != _selectedFixtures.end()) {
    _selectedFixtures.erase(iter, _selectedFixtures.end());
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
        const glight::theatre::FixtureMode &mode = fixture->Mode();
        const size_t shape_count = mode.Type().ShapeCount();
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
  const DryModeStyle dry_mode = context_menu_.GetDryModeStyle();
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

void VisualizationWidget::onLeftButtonPress(int, double x, double y) {
  auto modifiers = left_gesture_->get_current_event_state();
  const bool shift = (modifiers & Gdk::ModifierType::SHIFT_MASK) ==
                     Gdk::ModifierType::SHIFT_MASK;
  const std::optional<DrawInfo> info = GetPrimaryStyleDimensions(
      context_menu_.GetDryModeStyle(), get_width(), get_height());
  if (info) {
    const theatre::Coordinate2D pos =
        render_engine_.MouseToPosition(x, y, info->width, info->height);
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

void VisualizationWidget::onRightButtonPress(int, double x, double y) {
  queue_draw();
  context_menu_.SetSensitivity(Instance::State().LayoutLocked(),
                               _selectedFixtures.size());
  context_menu_.set_pointing_to(Gdk::Rectangle(x, y, 1, 1));
  context_menu_.popup();
}

void VisualizationWidget::onLeftButtonRelease(int, double x, double y) {
  if (_dragType == MouseState::DragFixture) {
    const std::optional<DrawInfo> info = GetPrimaryStyleDimensions(
        context_menu_.GetDryModeStyle(), get_width(), get_height());
    if (info) {
      _draggingStart =
          render_engine_.MouseToPosition(x, y, info->width, info->height);
    }
  }
  if (_dragType == MouseState::Track || _dragType == MouseState::TrackPan ||
      _dragType == MouseState::RotateFixture) {
    _dragType = MouseState::Normal;
  } else {
    _globalSelection->SetSelection(_selectedFixtures);
    _dragType = MouseState::Normal;
    _dragInvolvedFixtures.clear();
    queue_draw();
  }
}

void VisualizationWidget::SetCursor(const std::string &cursor_name) {
  if (cursor_name_ != cursor_name) {
    cursor_name_ = cursor_name;
    set_cursor(Gdk::Cursor::create(cursor_name));
  }
}

void VisualizationWidget::onMotion(double x, double y) {
  const std::optional<DrawInfo> info = GetPrimaryStyleDimensions(
      context_menu_.GetDryModeStyle(), get_width(), get_height());
  if (info) {
    const theatre::Coordinate2D pos =
        render_engine_.MouseToPosition(x, y, info->width, info->height);
    switch (_dragType) {
      case MouseState::Normal: {
        std::string cursor = "arrow";
        system::ObservingPtr<theatre::Fixture> hover_fixture =
            render_engine_.FixtureAt(pos);
        if (!hover_fixture) {
          system::ObservingPtr<theatre::Fixture> hover_handle =
              render_engine_.GetDirectionHandleAt(_selectedFixtures, pos);
          if (hover_handle) {
            cursor = "cross";
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
      case MouseState::Track:
        SetPan(pos);
        SetTilt(pos);
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
  sub_window_ = std::make_unique<windows::AddFixtureWindow>();
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
  designWizard->present();
}

void VisualizationWidget::onFixtureProperties() {
  if (!_propertiesWindow) {
    _propertiesWindow = system::MakeDeletable<windows::FixtureProperties>();
  }
  _propertiesWindow->present();
}

void VisualizationWidget::onSaveImage() {
  dialog_ = std::make_unique<Gtk::FileChooserDialog>(
      "Save image", Gtk::FileChooser::Action::SAVE);
  Gtk::FileChooserDialog &dialog =
      static_cast<Gtk::FileChooserDialog &>(*dialog_);
  dialog.add_button("Cancel", Gtk::ResponseType::CANCEL);
  dialog.add_button("Save", Gtk::ResponseType::OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("Scalable Vector Graphics (*.svg)");
  filter->add_pattern("*.svg");
  filter->add_mime_type("image/svg+xml");
  dialog.add_filter(filter);

  dialog.signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      Gtk::FileChooserDialog &dialog =
          static_cast<Gtk::FileChooserDialog &>(*dialog_);
      Glib::ustring filename(dialog.get_file()->get_path());
      if (filename.find('.') == Glib::ustring::npos) filename += ".svg";

      const size_t width = get_width();
      const size_t height = get_height();
      const Cairo::RefPtr<Cairo::SvgSurface> surface =
          Cairo::SvgSurface::create(filename, width, height);
      const Cairo::RefPtr<Cairo::Context> cairo =
          Cairo::Context::create(surface);
      DrawShapshot(cairo, {}, width, height);
      cairo->show_page();
      surface->finish();
      dialog_.reset();
    }
  });
  dialog.show();
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
  OpenColorDialog(dialog_, *main_window_, theatre::Color::RedC(),
                  [this](theatre::Color color) {
                    theatre::SetAllFixtures(*_management, _selectedFixtures,
                                            color);
                  });
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

void VisualizationWidget::OnTrack() { _dragType = MouseState::Track; }

void VisualizationWidget::OnTrackWithPan() { _dragType = MouseState::TrackPan; }

void VisualizationWidget::SetTilt(const theatre::Coordinate2D &position) {
  theatre::Management &management = Instance::Management();
  bool is_changed = false;
  for (const system::ObservingPtr<theatre::Fixture> &fixture :
       _selectedFixtures) {
    if (fixture->Mode().Type().CanBeamTilt()) {
      constexpr theatre::Coordinate2D offset(0.5, 0.5);
      const theatre::Coordinate2D direction =
          position - fixture->GetXY() - offset;
      const double z = fixture->GetPosition().Z();
      const double dist = std::sqrt(direction.X() * direction.X() +
                                    direction.Y() * direction.Y());
      double tilt = std::atan(z / dist) - fixture->StaticTilt();
      if (fixture->IsUpsideDown()) tilt = -tilt;
      const double begin_tilt = fixture->Mode().Type().MinTilt();
      const double end_tilt = fixture->Mode().Type().MaxTilt();
      const double min_value = std::min(begin_tilt, end_tilt);
      const double max_value = std::max(begin_tilt, end_tilt);
      tilt = system::RadialClamp(tilt, min_value, max_value);
      const double tilt_scaling = (tilt - begin_tilt) / (end_tilt - begin_tilt);
      theatre::FixtureControl &control =
          *management.GetFixtureControl(*fixture);
      for (size_t i = 0; i != control.NInputs(); ++i) {
        if (control.InputType(i) == theatre::FunctionType::Tilt) {
          theatre::SourceValue *source = management.GetSourceValue(control, i);
          source->A().Set(
              theatre::ControlValue::FromRatio(tilt_scaling).UInt());
          is_changed = true;
        }
      }
    }
  }
  if (is_changed) Update();
}

void VisualizationWidget::SetPan(const theatre::Coordinate2D &position) {
  theatre::Management &management = Instance::Management();
  bool is_changed = false;
  for (const system::ObservingPtr<theatre::Fixture> &fixture :
       _selectedFixtures) {
    if (fixture->Mode().Type().CanBeamRotate()) {
      constexpr theatre::Coordinate2D offset(0.5, 0.5);
      const theatre::Coordinate2D direction =
          position - fixture->GetXY() - offset;
      const bool is_zero = direction.Y() == 0.0 && direction.X() == 0.0;
      const double angle =
          is_zero ? 0.0 : std::atan2(direction.Y(), direction.X());
      double begin_pan = fixture->Mode().Type().MinPan();
      double end_pan = fixture->Mode().Type().MaxPan();
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
