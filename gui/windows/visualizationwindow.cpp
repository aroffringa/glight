#include "visualizationwindow.h"

#include "addfixturewindow.h"
#include "showwindow.h"

#include "../designwizard.h"
#include "../eventtransmitter.h"

#include "../../theatre/dmxdevice.h"
#include "../../theatre/fixture.h"
#include "../../theatre/management.h"
#include "../../theatre/theatre.h"
#include "../../theatre/valuesnapshot.h"

#include <glibmm/main.h>
#include <gtkmm/main.h>

namespace glight::gui {

VisualizationWindow::VisualizationWindow(theatre::Management *management,
                                         EventTransmitter *eventTransmitter,
                                         FixtureSelection *fixtureSelection,
                                         ShowWindow *showWindow)
    : _management(management),
      _eventTransmitter(eventTransmitter),
      _globalSelection(fixtureSelection),
      _showWindow(showWindow),
      _isInitialized(false),
      _isTimerRunning(false),
      _dragType(NotDragging),
      _selectedFixtures(),
      _miSymbolMenu("Symbol"),
      _miDryModeStyle("Dry mode style"),
      _miAlignHorizontally("Align horizontally"),
      _miAlignVertically("Align vertically"),
      _miDistributeEvenly("Distribute evenly"),
      _miAdd("Add..."),
      _miRemove("Remove"),
      _miDesign("Design..."),
      _miFullscreen("Fullscreen"),
      _miDMSSecondary("Single"),
      _miDMSVertical("Vertical"),
      _miDMSHorizontal("Horizontal"),
      _miDMSShadow("Shadow") {
  set_title("Glight - visualization");
  set_default_size(600, 200);

  _globalSelectionConnection = _globalSelection->SignalChange().connect(
      [&]() { onGlobalSelectionChanged(); });

  _drawingArea.set_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                          Gdk::POINTER_MOTION_MASK);
  _drawingArea.signal_draw().connect(
      sigc::mem_fun(*this, &VisualizationWindow::onExpose));
  _drawingArea.signal_button_press_event().connect(
      sigc::mem_fun(*this, &VisualizationWindow::onButtonPress));
  _drawingArea.signal_button_release_event().connect(
      sigc::mem_fun(*this, &VisualizationWindow::onButtonRelease));
  _drawingArea.signal_motion_notify_event().connect(
      sigc::mem_fun(*this, &VisualizationWindow::onMotion));
  add(_drawingArea);
  _drawingArea.show();
  inializeContextMenu();
}

VisualizationWindow::~VisualizationWindow() {
  _timeoutConnection.disconnect();
  _globalSelectionConnection.disconnect();
}

void VisualizationWindow::inializeContextMenu() {
  std::vector<theatre::FixtureSymbol::Symbol> symbols(
      theatre::FixtureSymbol::List());
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
  _dryModeStyleMenu.add(_miDMSPrimary);
  _miDMSSecondary.set_group(dryModeStyleGroup);
  _dryModeStyleMenu.add(_miDMSSecondary);
  _miDMSHorizontal.set_group(dryModeStyleGroup);
  _dryModeStyleMenu.add(_miDMSHorizontal);
  _miDMSVertical.set_group(dryModeStyleGroup);
  _miDMSVertical.set_active(true);
  _dryModeStyleMenu.add(_miDMSVertical);
  _miDMSShadow.set_group(dryModeStyleGroup);
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

  _miRemove.signal_activate().connect([&] { onRemoveFixtures(); });
  _popupMenu.add(_miRemove);

  _miDesign.signal_activate().connect([&] { onDesignFixtures(); });
  _popupMenu.add(_miDesign);

  _popupMenu.add(_miSeparator2);

  _miFullscreen.signal_activate().connect([&] { onFullscreen(); });
  _popupMenu.add(_miFullscreen);

  _popupMenu.show_all_children();
}

void VisualizationWindow::initialize() {
  queue_draw();
  _isInitialized = true;

  if (!_isTimerRunning) {
    _timeoutConnection = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &VisualizationWindow::onTimeout), 40);
    _isTimerRunning = true;
  }
}

void VisualizationWindow::onTheatreChanged() {
  for (size_t i = _selectedFixtures.size(); i != 0; --i) {
    if (!_management->GetTheatre().Contains(*_selectedFixtures[i - 1]))
      _selectedFixtures.erase(_selectedFixtures.begin() + i - 1);
  }
  Update();
}

bool VisualizationWindow::onExpose(
    const Cairo::RefPtr<Cairo::Context> &context) {
  if (!_isInitialized) initialize();

  drawAll(context);
  return true;
}

double VisualizationWindow::scale(double width,
                                  double height) const {
  const theatre::Theatre &theatre = _management->GetTheatre();
  theatre::Position extend = theatre.Extend();
  if (extend.X() == 0.0 || extend.Y() == 0.0)
    return 1.0;
  else
    return std::min(width / extend.X(), height / extend.Y());
}

void VisualizationWindow::drawManagement(
    const Cairo::RefPtr<Cairo::Context> &cairo, const theatre::ValueSnapshot& snapshot,
    const DrawStyle &style) {
  const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
      _management->GetTheatre().Fixtures();
  cairo->save();
  double sc = scale(style.width, style.height);
  cairo->scale(sc, sc);

  _fixtureStates.resize(fixtures.size());
  for (size_t fixtureIndex = 0; fixtureIndex != fixtures.size();
       ++fixtureIndex) {
    const theatre::Fixture &fixture = *fixtures[fixtureIndex];
    FixtureState &state = _fixtureStates[fixtureIndex];
    if (fixture.IsVisible()) {
      size_t shapeCount = fixture.Type().ShapeCount();
      for (size_t i = 0; i != shapeCount; ++i) {
        const size_t shapeIndex = shapeCount - i - 1;
        const Color c = fixture.GetColor(snapshot, shapeIndex);

        cairo->set_source_rgb((double)c.Red() / 224.0 + 0.125,
                              (double)c.Green() / 224.0 + 0.125,
                              (double)c.Blue() / 224.0 + 0.125);

        const double singleRadius = radius(fixture.Symbol().Value());
        const double radius =
            shapeCount == 1
                ? singleRadius
                : 0.33 + 0.07 * double(shapeIndex) / (shapeCount - 1);
        const double x = fixture.GetPosition().X() + 0.5;
        const double y = fixture.GetPosition().Y() + 0.5;
        cairo->arc(x + style.xOffset / sc, y + style.yOffset / sc, radius, 0.0,
                   2.0 * M_PI);
        cairo->fill();

        const int rotation = fixture.GetRotationSpeed(snapshot, shapeIndex);
        if (rotation != 0) {
          const double rotationDisp =
              M_PI * double(rotation) * style.timeSince / (10.0 * (1 << 24));
          state.rotation = std::fmod(rotationDisp + state.rotation, M_PI);
          const double s = std::sin(state.rotation);
          const double c = std::cos(state.rotation);
          cairo->set_line_width(radius * 0.2);
          cairo->set_source_rgb(0, 0, 0);
          cairo->move_to(x + c * radius + style.xOffset / sc,
                         y + s * radius + style.yOffset / sc);
          cairo->line_to(x - c * radius + style.xOffset / sc,
                         y - s * radius + style.yOffset / sc);
          cairo->move_to(x + s * radius + style.xOffset / sc,
                         y - c * radius + style.yOffset / sc);
          cairo->line_to(x - s * radius + style.xOffset / sc,
                         y + c * radius + style.yOffset / sc);
          cairo->stroke();
        }
      }
    }
  }
  cairo->set_source_rgb(0.2, 0.2, 1.0);
  cairo->set_line_width(4.0 / sc);
  for (const theatre::Fixture *f : _selectedFixtures) {
    if (f->IsVisible()) {
      double rad = radius(f->Symbol().Value());
      double x = f->GetPosition().X() + 0.5;
      double y = f->GetPosition().Y() + 0.5;
      cairo->arc(x + style.xOffset / sc, y + style.yOffset / sc, rad, 0.0,
                 2.0 * M_PI);
      cairo->stroke();
    }
  }

  if (_dragType == DragRectangle || _dragType == DragAddRectangle) {
    std::pair<double, double> size = _draggingTo - _draggingStart;
    cairo->rectangle(_draggingStart.X(), _draggingStart.Y(), size.first,
                     size.second);
    cairo->set_source_rgba(0.2, 0.2, 1.0, 0.5);
    cairo->fill_preserve();
    cairo->set_source_rgba(0.5, 0.5, 1.0, 0.8);
    cairo->set_line_width(2.0 / sc);
    cairo->stroke();
  }

  cairo->restore();
}

void VisualizationWindow::drawAll(const Cairo::RefPtr<Cairo::Context> &cairo) {
  size_t width = _drawingArea.get_width(), height = _drawingArea.get_height();

  cairo->set_source_rgba(0, 0, 0, 1);
  cairo->rectangle(0, 0, width, height);
  cairo->fill();

  const double time = _management->GetOffsetTimeInMS();
  static double previousTime = time;

  DrawStyle style{.xOffset = 0,
                  .yOffset = 0,
                  .width = width,
                  .height = height,
                  .timeSince = time - previousTime};
  previousTime = time;
  if (_miDMSPrimary.get_active()) {
    drawManagement(cairo, _management->PrimarySnapshot(), style);
  } else if (_miDMSHorizontal.get_active()) {
    style.xOffset = 0;
    style.width = width / 2;
    drawManagement(cairo, _management->PrimarySnapshot(), style);
    style.xOffset = style.width;
    style.width = width - style.width;
    drawManagement(cairo, _management->SecondarySnapshot(), style);
  } else if (_miDMSVertical.get_active()) {
    style.yOffset = 0;
    style.height = height / 2;
    drawManagement(cairo, _management->PrimarySnapshot(), style);
    style.yOffset = style.height;
    style.height = height - style.height;
    drawManagement(cairo, _management->SecondarySnapshot(), style);
  } else if (_miDMSShadow.get_active()) {
    style.xOffset = width * 1 / 100;
    style.yOffset = height * 1 / 100;
    style.width = width * 99 / 100;
    style.height = height * 99 / 100;
    drawManagement(cairo, _management->PrimarySnapshot(), style);
    style.xOffset = 0;
    style.yOffset = 0;
    drawManagement(cairo, _management->SecondarySnapshot(), style);
  } else {  // Secondary
    drawManagement(cairo, _management->SecondarySnapshot(), style);
  }
}

theatre::Fixture *VisualizationWindow::fixtureAt(
    theatre::Management &management, const theatre::Position &position) {
  const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
      management.GetTheatre().Fixtures();

  theatre::Fixture *fix = nullptr;
  double closest = std::numeric_limits<double>::max();
  for (const std::unique_ptr<theatre::Fixture> &f : fixtures) {
    if (f->IsVisible() &&
        position.InsideRectangle(f->GetPosition(),
                                 f->GetPosition().Add(1.0, 1.0))) {
      double distanceSq =
          position.SquaredDistance(f->GetPosition().Add(0.5, 0.5));
      double radSq = radiusSq(f->Symbol().Value());
      if (distanceSq <= radSq && distanceSq < closest) {
        fix = f.get();
        closest = distanceSq;
      }
    }
  }
  return fix;
}

bool VisualizationWindow::onButtonPress(GdkEventButton *event) {
  if (event->button == 1 || event->button == 3) {
    const bool shift =
        (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) ==
        GDK_SHIFT_MASK;
    double height;
    if (_miDMSPrimary.get_active())
      height = _drawingArea.get_height();
    else // TODO other dry mode styles
      height = _drawingArea.get_height() / 2.0;
    const double sc = invScale(_drawingArea.get_width(), height);
    const theatre::Position pos = theatre::Position(event->x, event->y) * sc;
    theatre::Fixture *selectedFixture = fixtureAt(*_management, pos);
    if (!shift) {
      if (selectedFixture) {
        // Was a fixture clicked that was already selected? Then keep all
        // selected. If not, select the clicked fixture:
        if (std::find(_selectedFixtures.begin(), _selectedFixtures.end(),
                      selectedFixture) == _selectedFixtures.end()) {
          _selectedFixtures.assign(1, selectedFixture);
        }
      } else {
        _selectedFixtures.clear();
      }
    }

    if (event->button == 1) {
      _draggingStart = pos;
      if (shift) {
        _dragType = DragAddRectangle;
        _draggingTo = pos;
        _selectedFixturesBeforeDrag = _selectedFixtures;
      } else if (!_selectedFixtures.empty()) {
        _dragType = DragFixture;
      } else {
        _dragType = DragRectangle;
        _draggingTo = pos;
      }
      queue_draw();
    } else if (event->button == 3) {
      queue_draw();
      _miAlignHorizontally.set_sensitive(_selectedFixtures.size() >= 2);
      _miAlignVertically.set_sensitive(_selectedFixtures.size() >= 2);
      _miDistributeEvenly.set_sensitive(_selectedFixtures.size() >= 2);
      _miRemove.set_sensitive(_selectedFixtures.size() >= 1);
      _miSymbolMenu.set_sensitive(_selectedFixtures.size() >= 1);
      _popupMenu.popup(event->button, event->time);
    }
  }
  return true;
}

bool VisualizationWindow::onButtonRelease(GdkEventButton *event) {
  if (event->button == 1) {
    const double sc = invScale(_drawingArea.get_width(),
                         _drawingArea.get_height());
    if (_dragType == DragFixture) {
      _draggingStart = theatre::Position(event->x, event->y) * sc;
    } else if (_dragType == DragRectangle || _dragType == DragAddRectangle) {
    }
    _globalSelection->SetSelection(_selectedFixtures);
    _dragType = NotDragging;
    _selectedFixturesBeforeDrag.clear();
    queue_draw();
  }
  return true;
}

bool VisualizationWindow::onMotion(GdkEventMotion *event) {
  if (_dragType != NotDragging) {
    const double width = _drawingArea.get_width();
    const double height = _drawingArea.get_height();
    const theatre::Position pos = theatre::Position(event->x, event->y) /
                            scale(width, height);
    switch (_dragType) {
      case NotDragging:
        break;
      case DragFixture:
        for (theatre::Fixture *fixture : _selectedFixtures)
          fixture->GetPosition() += pos - _draggingStart;
        _draggingStart = pos;
        break;
      case DragRectangle:
        _draggingTo = pos;
        selectFixtures(_draggingStart, _draggingTo);
        break;
      case DragAddRectangle:
        _draggingTo = pos;
        addFixtures(_draggingStart, _draggingTo);
        break;
    }
    queue_draw();
  }
  return true;
}

void VisualizationWindow::selectFixtures(const theatre::Position &a,
                                         const theatre::Position &b) {
  _selectedFixtures.clear();
  double x1 = a.X(), y1 = a.Y(), x2 = b.X(), y2 = b.Y();
  if (x1 > x2) std::swap(x1, x2);
  if (y1 > y2) std::swap(y1, y2);
  theatre::Position first(x1 - 0.1, y1 - 0.1);
  theatre::Position second(x2 - 0.9, y2 - 0.9);
  if (second.X() - first.X() > 0.0 && second.Y() - first.Y() > 0.0) {
    const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
        _management->GetTheatre().Fixtures();
    for (const std::unique_ptr<theatre::Fixture> &fixture : fixtures) {
      if (fixture->IsVisible() &&
          fixture->GetPosition().InsideRectangle(first, second))
        _selectedFixtures.emplace_back(fixture.get());
    }
  }
}

void VisualizationWindow::addFixtures(const theatre::Position &a,
                                      const theatre::Position &b) {
  selectFixtures(a, b);
  for (theatre::Fixture *fixture : _selectedFixturesBeforeDrag) {
    auto iter =
        std::find(_selectedFixtures.begin(), _selectedFixtures.end(), fixture);
    if (iter == _selectedFixtures.end())
      _selectedFixtures.emplace_back(fixture);
    else
      _selectedFixtures.erase(iter);
  }
}

void VisualizationWindow::onAlignHorizontally() {
  if (_selectedFixtures.size() >= 2) {
    double y = 0.0;

    for (const theatre::Fixture *fixture : _selectedFixtures)
      y += fixture->GetPosition().Y();

    y /= _selectedFixtures.size();

    for (theatre::Fixture *fixture : _selectedFixtures)
      fixture->GetPosition().Y() = y;
  }
}

void VisualizationWindow::onAlignVertically() {
  if (_selectedFixtures.size() >= 2) {
    double x = 0.0;

    for (const theatre::Fixture *fixture : _selectedFixtures)
      x += fixture->GetPosition().X();

    x /= _selectedFixtures.size();

    for (theatre::Fixture *fixture : _selectedFixtures)
      fixture->GetPosition().X() = x;
  }
}

void VisualizationWindow::onDistributeEvenly() {
  if (_selectedFixtures.size() >= 2) {
    double left = _selectedFixtures[0]->GetPosition().X();
    double right = _selectedFixtures[0]->GetPosition().X();
    double top = _selectedFixtures[0]->GetPosition().Y();
    double bottom = _selectedFixtures[0]->GetPosition().Y();

    for (size_t i = 1; i != _selectedFixtures.size(); ++i) {
      const theatre::Fixture *fixture = _selectedFixtures[i];
      left = std::min(fixture->GetPosition().X(), left);
      right = std::max(fixture->GetPosition().X(), right);
      top = std::min(fixture->GetPosition().Y(), top);
      bottom = std::max(fixture->GetPosition().Y(), bottom);
    }

    std::vector<theatre::Fixture *> list = _selectedFixtures;
    if (left == right) {
      std::sort(list.begin(), list.end(),
                [](const theatre::Fixture *a, const theatre::Fixture *b) {
                  return a->GetPosition().Y() < b->GetPosition().Y();
                });
      for (size_t i = 0; i != list.size(); ++i) {
        double y = double(i) / double(list.size() - 1) * (bottom - top) + top;
        list[i]->GetPosition().Y() = y;
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
        double r = double(i) / double(list.size() - 1);
        double x = r * (right - left) + left;
        double y = r * (bottom - top) + top;
        list[i]->GetPosition() = theatre::Position(x, y);
      }
    }
  }
}

void VisualizationWindow::onAddFixtures() {
  AddFixtureWindow window(_eventTransmitter, *_management);
  window.set_modal(true);
  Gtk::Main::run(window);
}

void VisualizationWindow::onRemoveFixtures() {
  for (theatre::Fixture *fixture : _selectedFixtures) {
    _management->RemoveFixture(*fixture);
  }
  _selectedFixtures.clear();
  _eventTransmitter->EmitUpdate();
}

void VisualizationWindow::onDesignFixtures() {
  std::unique_ptr<DesignWizard> &designWizard = _showWindow->GetDesignWizard();
  designWizard.reset(
      new DesignWizard(*_management, *_eventTransmitter, _showWindow->Path()));
  designWizard->Select(_selectedFixtures);
  designWizard->present();
}

void VisualizationWindow::onFullscreen() {
  if (_miFullscreen.get_active())
    fullscreen();
  else
    unfullscreen();
}

void VisualizationWindow::onSetSymbol(theatre::FixtureSymbol::Symbol symbol) {
  for (theatre::Fixture *fixture : _selectedFixtures) {
    fixture->SetSymbol(theatre::FixtureSymbol(symbol));
  }
  if (symbol == theatre::FixtureSymbol::Hidden) _selectedFixtures.clear();
  _eventTransmitter->EmitUpdate();
}

void VisualizationWindow::onGlobalSelectionChanged() {
  _selectedFixtures = _globalSelection->Selection();
  queue_draw();
}

}  // namespace glight::gui
