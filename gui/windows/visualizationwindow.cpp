#include "visualizationwindow.h"

#include "addfixturewindow.h"
#include "fixtureproperties.h"
#include "mainwindow.h"

#include "../designwizard.h"
#include "../eventtransmitter.h"

#include "../../theatre/dmxdevice.h"
#include "../../theatre/fixture.h"
#include "../../theatre/fixturegroup.h"
#include "../../theatre/management.h"
#include "../../theatre/theatre.h"
#include "../../theatre/valuesnapshot.h"

#include <glibmm/main.h>
#include <gtkmm/main.h>
#include <cmath>

#include <memory>

namespace glight::gui {

VisualizationWindow::VisualizationWindow(theatre::Management *management,
                                         EventTransmitter *eventTransmitter,
                                         FixtureSelection *fixtureSelection,
                                         MainWindow *showWindow)
    : _management(management),
      _eventTransmitter(eventTransmitter),
      _globalSelection(fixtureSelection),
      _showWindow(showWindow),
      _isInitialized(false),
      _isTimerRunning(false),
      _dragType(NotDragging),

      _renderEngine(*management),
      _miSymbolMenu("Symbol"),
      _miDryModeStyle("Dry mode style"),
      _miAlignHorizontally("Align horizontally"),
      _miAlignVertically("Align vertically"),
      _miDistributeEvenly("Distribute evenly"),
      _miAdd("Add..."),
      _miRemove("Remove"),
      _miGroup("Group..."),
      _miDesign("Design..."),
      _miFullscreen("Fullscreen"),
      _miProperties("Properties"),
      _miDMSPrimary("Primary"),
      _miDMSSecondary("Secondary"),
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
  _miDMSPrimary.set_active(true);
  _dryModeStyleMenu.add(_miDMSPrimary);
  _miDMSSecondary.set_group(dryModeStyleGroup);
  _dryModeStyleMenu.add(_miDMSSecondary);
  _miDMSHorizontal.set_group(dryModeStyleGroup);
  _dryModeStyleMenu.add(_miDMSHorizontal);
  _miDMSVertical.set_group(dryModeStyleGroup);
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

  _miGroup.signal_activate().connect([&] { onGroupFixtures(); });
  _popupMenu.add(_miGroup);

  _miDesign.signal_activate().connect([&] { onDesignFixtures(); });
  _popupMenu.add(_miDesign);

  _popupMenu.add(_miSeparator2);

  _miFullscreen.signal_activate().connect([&] { onFullscreen(); });
  _popupMenu.add(_miFullscreen);

  _miProperties.signal_activate().connect([&] { onFixtureProperties(); });
  _popupMenu.add(_miProperties);

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

void VisualizationWindow::drawAll(const Cairo::RefPtr<Cairo::Context> &cairo) {
  const size_t width = _drawingArea.get_width();
  const size_t height = _drawingArea.get_height();

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
    _renderEngine.DrawSnapshot(cairo, _management->PrimarySnapshot(), style,
                               _selectedFixtures);
  } else if (_miDMSHorizontal.get_active()) {
    style.xOffset = 0;
    style.width = width / 2;
    _renderEngine.DrawSnapshot(cairo, _management->PrimarySnapshot(), style,
                               _selectedFixtures);
    style.xOffset = style.width;
    style.width = width - style.width;
    _renderEngine.DrawSnapshot(cairo, _management->SecondarySnapshot(), style,
                               _selectedFixtures);
  } else if (_miDMSVertical.get_active()) {
    style.yOffset = 0;
    style.height = height / 2;
    _renderEngine.DrawSnapshot(cairo, _management->PrimarySnapshot(), style,
                               _selectedFixtures);
    style.yOffset = style.height;
    style.height = height - style.height;
    _renderEngine.DrawSnapshot(cairo, _management->SecondarySnapshot(), style,
                               _selectedFixtures);
  } else if (_miDMSShadow.get_active()) {
    style.xOffset = width * 1 / 100;
    style.yOffset = height * 1 / 100;
    style.width = width * 99 / 100;
    style.height = height * 99 / 100;
    _renderEngine.DrawSnapshot(cairo, _management->PrimarySnapshot(), style,
                               _selectedFixtures);
    style.xOffset = 0;
    style.yOffset = 0;
    _renderEngine.DrawSnapshot(cairo, _management->SecondarySnapshot(), style,
                               _selectedFixtures);
  } else {  // Secondary
    _renderEngine.DrawSnapshot(cairo, _management->SecondarySnapshot(), style,
                               _selectedFixtures);
  }

  if (_dragType == DragRectangle || _dragType == DragAddRectangle) {
    _renderEngine.DrawSelectionRectangle(cairo, _draggingStart, _draggingTo);
  }
}

bool VisualizationWindow::onButtonPress(GdkEventButton *event) {
  if (event->button == 1 || event->button == 3) {
    const bool shift =
        (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) ==
        GDK_SHIFT_MASK;
    double height;
    if (_miDMSPrimary.get_active())
      height = _drawingArea.get_height();
    else  // TODO other dry mode styles
      height = _drawingArea.get_height() / 2.0;
    const theatre::Position pos = _renderEngine.MouseToPosition(
        event->x, event->y, _drawingArea.get_width(), height);
    theatre::Fixture *selectedFixture = _renderEngine.FixtureAt(pos);
    if (!shift) {
      if (selectedFixture) {
        // Was a fixture clicked that was already selected? Then keep all
        // selected. If not, select the clicked fixture:
        if (std::find(_selectedFixtures.begin(), _selectedFixtures.end(),
                      selectedFixture) == _selectedFixtures.end()) {
          _selectedFixtures.assign(1, selectedFixture);
          _globalSelection->SetSelection(_selectedFixtures);
        }
      } else {
        _selectedFixtures.clear();
        _globalSelection->SetSelection(_selectedFixtures);
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
      _miRemove.set_sensitive(!_selectedFixtures.empty());
      _miSymbolMenu.set_sensitive(!_selectedFixtures.empty());
      _miProperties.set_sensitive(!_selectedFixtures.empty());
      _popupMenu.popup_at_pointer(reinterpret_cast<GdkEvent *>(event));
    }
  }
  return true;
}

bool VisualizationWindow::onButtonRelease(GdkEventButton *event) {
  if (event->button == 1) {
    if (_dragType == DragFixture) {
      // TODO correct width/height for layout
      _draggingStart = _renderEngine.MouseToPosition(event->x, event->y,
                                                     _drawingArea.get_width(),
                                                     _drawingArea.get_height());
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
    const theatre::Position pos =
        _renderEngine.MouseToPosition(event->x, event->y, width, height);
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
  double x1 = a.X();
  double y1 = a.Y();
  double x2 = b.X();
  double y2 = b.Y();
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
        double y = static_cast<double>(i) /
                       static_cast<double>(list.size() - 1) * (bottom - top) +
                   top;
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
        double r =
            static_cast<double>(i) / static_cast<double>(list.size() - 1);
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
  window.set_transient_for(*this);
  window.show();
}

void VisualizationWindow::onRemoveFixtures() {
  for (theatre::Fixture *fixture : _selectedFixtures) {
    std::lock_guard<std::mutex> lock(_management->Mutex());
    _management->RemoveFixture(*fixture);
  }
  _selectedFixtures.clear();
  _eventTransmitter->EmitUpdate();
}

void VisualizationWindow::onGroupFixtures() {
  std::unique_lock lock(_management->Mutex());
  theatre::Folder &parent = _showWindow->SelectedFolder();
  const std::string name = parent.GetAvailableName("group");
  theatre::FixtureGroup &group = _management->AddFixtureGroup(parent, name);
  for (theatre::Fixture *fixture : _selectedFixtures) {
    group.Insert(*fixture);
  }
  lock.unlock();
  _eventTransmitter->EmitUpdate();
  _showWindow->OpenPropertiesWindow(group);
}

void VisualizationWindow::onDesignFixtures() {
  std::unique_ptr<DesignWizard> &designWizard = _showWindow->GetDesignWizard();
  designWizard =
      std::make_unique<DesignWizard>(*_management, *_eventTransmitter);
  designWizard->SetCurrentPath(_showWindow->SelectedFolder().FullPath());
  designWizard->Select(_selectedFixtures);
  designWizard->present();
}

void VisualizationWindow::onFullscreen() {
  if (_miFullscreen.get_active())
    fullscreen();
  else
    unfullscreen();
}

void VisualizationWindow::onFixtureProperties() {
  if (!_propertiesWindow) {
    _propertiesWindow = system::MakeDeletable<windows::FixtureProperties>(
        *_eventTransmitter, *_management, *_globalSelection);
  }
  _propertiesWindow->present();
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
