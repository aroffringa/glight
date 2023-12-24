#include "visualizationwidget.h"

#include "../windows/addfixturewindow.h"
#include "../windows/fixtureproperties.h"
#include "../windows/mainwindow.h"

#include "../designwizard.h"
#include "../eventtransmitter.h"

#include "../../theatre/dmxdevice.h"
#include "../../theatre/fixture.h"
#include "../../theatre/fixturegroup.h"
#include "../../theatre/management.h"
#include "../../theatre/theatre.h"
#include "../../theatre/valuesnapshot.h"

#include <glibmm/main.h>

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/main.h>

#include <cmath>
#include <memory>

namespace glight::gui {

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
      _dragType(NotDragging),

      render_engine_(*management),
      _miSymbolMenu("Symbol"),
      _miDryModeStyle("Dry mode style"),
      _miAlignHorizontally("Align horizontally"),
      _miAlignVertically("Align vertically"),
      _miDistributeEvenly("Distribute evenly"),
      _miAdd("Add..."),
      _miRemove("Remove"),
      _miGroup("Group..."),
      _miDesign("Design..."),
      _miProperties("Properties"),
      _miSaveImage("Save image..."),
      _miDMSPrimary("Primary"),
      _miDMSSecondary("Secondary"),
      _miDMSVertical("Vertical"),
      _miDMSHorizontal("Horizontal"),
      _miDMSShadow("Shadow") {
  set_size_request(600, 200);

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
}

VisualizationWidget::~VisualizationWidget() {
  _timeoutConnection.disconnect();
  _globalSelectionConnection.disconnect();
}

void VisualizationWidget::inializeContextMenu() {
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

void VisualizationWidget::drawFixtures(
    const Cairo::RefPtr<Cairo::Context> &cairo,
    const std::vector<theatre::Fixture *> &selection, size_t width,
    size_t height) {
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
    render_engine_.DrawSnapshot(cairo, _management->PrimarySnapshot(), style,
                                selection);
  } else if (_miDMSHorizontal.get_active()) {
    style.xOffset = 0;
    style.width = width / 2;
    render_engine_.DrawSnapshot(cairo, _management->PrimarySnapshot(), style,
                                selection);
    style.xOffset = style.width;
    style.width = width - style.width;
    render_engine_.DrawSnapshot(cairo, _management->SecondarySnapshot(), style,
                                selection);
  } else if (_miDMSVertical.get_active()) {
    style.yOffset = 0;
    style.height = height / 2;
    render_engine_.DrawSnapshot(cairo, _management->PrimarySnapshot(), style,
                                selection);
    style.yOffset = style.height;
    style.height = height - style.height;
    render_engine_.DrawSnapshot(cairo, _management->SecondarySnapshot(), style,
                                selection);
  } else if (_miDMSShadow.get_active()) {
    style.xOffset = width * 1 / 100;
    style.yOffset = height * 1 / 100;
    style.width = width * 99 / 100;
    style.height = height * 99 / 100;
    render_engine_.DrawSnapshot(cairo, _management->PrimarySnapshot(), style,
                                selection);
    style.xOffset = 0;
    style.yOffset = 0;
    render_engine_.DrawSnapshot(cairo, _management->SecondarySnapshot(), style,
                                selection);
  } else {  // Secondary
    render_engine_.DrawSnapshot(cairo, _management->SecondarySnapshot(), style,
                                selection);
  }
}

void VisualizationWidget::drawAll(const Cairo::RefPtr<Cairo::Context> &cairo) {
  const size_t width = get_width();
  const size_t height = get_height();

  drawFixtures(cairo, _selectedFixtures, width, height);

  if (_dragType == DragRectangle || _dragType == DragAddRectangle) {
    render_engine_.DrawSelectionRectangle(cairo, _draggingStart, _draggingTo);
  }
}

bool VisualizationWidget::onButtonPress(GdkEventButton *event) {
  if (event->button == 1 || event->button == 3) {
    const bool shift =
        (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) ==
        GDK_SHIFT_MASK;
    double height;
    if (_miDMSPrimary.get_active())
      height = get_height();
    else  // TODO other dry mode styles
      height = get_height() / 2.0;
    const theatre::Position pos =
        render_engine_.MouseToPosition(event->x, event->y, get_width(), height);
    theatre::Fixture *selectedFixture = render_engine_.FixtureAt(pos);
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

bool VisualizationWidget::onButtonRelease(GdkEventButton *event) {
  if (event->button == 1) {
    if (_dragType == DragFixture) {
      // TODO correct width/height for layout
      _draggingStart = render_engine_.MouseToPosition(
          event->x, event->y, get_width(), get_height());
    } else if (_dragType == DragRectangle || _dragType == DragAddRectangle) {
    }
    _globalSelection->SetSelection(_selectedFixtures);
    _dragType = NotDragging;
    _selectedFixturesBeforeDrag.clear();
    queue_draw();
  }
  return true;
}

bool VisualizationWidget::onMotion(GdkEventMotion *event) {
  if (_dragType != NotDragging) {
    const double width = get_width();
    const double height = get_height();
    const theatre::Position pos =
        render_engine_.MouseToPosition(event->x, event->y, width, height);
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

void VisualizationWidget::selectFixtures(const theatre::Position &a,
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

void VisualizationWidget::addFixtures(const theatre::Position &a,
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

void VisualizationWidget::onAlignHorizontally() {
  if (_selectedFixtures.size() >= 2) {
    double y = 0.0;

    for (const theatre::Fixture *fixture : _selectedFixtures)
      y += fixture->GetPosition().Y();

    y /= _selectedFixtures.size();

    for (theatre::Fixture *fixture : _selectedFixtures)
      fixture->GetPosition().Y() = y;
  }
}

void VisualizationWidget::onAlignVertically() {
  if (_selectedFixtures.size() >= 2) {
    double x = 0.0;

    for (const theatre::Fixture *fixture : _selectedFixtures)
      x += fixture->GetPosition().X();

    x /= _selectedFixtures.size();

    for (theatre::Fixture *fixture : _selectedFixtures)
      fixture->GetPosition().X() = x;
  }
}

void VisualizationWidget::onDistributeEvenly() {
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

void VisualizationWidget::onAddFixtures() {
  sub_window_ =
      std::make_unique<AddFixtureWindow>(_eventTransmitter, *_management);
  sub_window_->set_transient_for(*main_window_);
  sub_window_->set_modal(true);
  sub_window_->show();
}

void VisualizationWidget::onRemoveFixtures() {
  for (theatre::Fixture *fixture : _selectedFixtures) {
    std::lock_guard<std::mutex> lock(_management->Mutex());
    _management->RemoveFixture(*fixture);
  }
  _selectedFixtures.clear();
  _eventTransmitter->EmitUpdate();
}

void VisualizationWidget::onGroupFixtures() {
  std::unique_lock lock(_management->Mutex());
  theatre::Folder &parent = main_window_->SelectedFolder();
  const std::string name = parent.GetAvailableName("group");
  theatre::FixtureGroup &group = _management->AddFixtureGroup(parent, name);
  for (theatre::Fixture *fixture : _selectedFixtures) {
    group.Insert(*fixture);
  }
  lock.unlock();
  _eventTransmitter->EmitUpdate();
  main_window_->OpenPropertiesWindow(group);
}

void VisualizationWidget::onDesignFixtures() {
  std::unique_ptr<DesignWizard> &designWizard = main_window_->GetDesignWizard();
  designWizard =
      std::make_unique<DesignWizard>(*_management, *_eventTransmitter);
  designWizard->SetCurrentPath(main_window_->SelectedFolder().FullPath());
  designWizard->Select(_selectedFixtures);
  designWizard->present();
}

void VisualizationWidget::onFixtureProperties() {
  if (!_propertiesWindow) {
    _propertiesWindow = system::MakeDeletable<windows::FixtureProperties>(
        *_eventTransmitter, *_management, *_globalSelection);
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
    drawFixtures(cairo, {}, width, height);
    cairo->show_page();
    surface->finish();
  }
}

void VisualizationWidget::onSetSymbol(theatre::FixtureSymbol::Symbol symbol) {
  for (theatre::Fixture *fixture : _selectedFixtures) {
    fixture->SetSymbol(theatre::FixtureSymbol(symbol));
  }
  if (symbol == theatre::FixtureSymbol::Hidden) _selectedFixtures.clear();
  _eventTransmitter->EmitUpdate();
}

void VisualizationWidget::onGlobalSelectionChanged() {
  _selectedFixtures = _globalSelection->Selection();
  queue_draw();
}

}  // namespace glight::gui
