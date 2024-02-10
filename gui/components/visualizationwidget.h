#ifndef GLIGHT_GUI_VISUALIZATION_WINDOW_H_
#define GLIGHT_GUI_VISUALIZATION_WINDOW_H_

#include "../../theatre/fixturesymbol.h"
#include "../../theatre/forwards.h"
#include "../../theatre/position.h"

#include "../renderengine.h"

#include "../../system/deletableptr.h"
#include "../../system/midicontroller.h"

#include <gdkmm/pixbuf.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/menu.h>
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/separatormenuitem.h>

namespace glight::gui {

class EventTransmitter;
class FixtureSelection;
class MainWindow;

namespace windows {
class FixtureProperties;
}

class VisualizationWidget : public Gtk::DrawingArea {
 public:
  VisualizationWidget(theatre::Management *management,
                      EventTransmitter *eventTransmitter,
                      FixtureSelection *fixtureSelection,
                      MainWindow *showWindow);
  ~VisualizationWidget();

  void Update() { queue_draw(); }

 private:
  VisualizationWidget(const VisualizationWidget &) = delete;
  VisualizationWidget &operator=(const VisualizationWidget &) = delete;

  theatre::Management *_management;
  EventTransmitter *_eventTransmitter;
  FixtureSelection *_globalSelection;
  sigc::connection _globalSelectionConnection;
  MainWindow *main_window_;
  system::DeletablePtr<glight::gui::windows::FixtureProperties>
      _propertiesWindow;
  bool _isInitialized, _isTimerRunning;
  sigc::connection _timeoutConnection;
  enum DragType {
    NotDragging,
    DragFixture,
    DragRectangle,
    DragAddRectangle
  } _dragType;
  std::vector<theatre::Fixture *> _selectedFixtures,
      _selectedFixturesBeforeDrag;
  theatre::Position _draggingStart, _draggingTo;
  RenderEngine render_engine_;
  std::unique_ptr<Gtk::Window> sub_window_;

  Gtk::Menu _popupMenu;
  Gtk::SeparatorMenuItem _miSeparator1, _miSeparator2;
  Gtk::MenuItem _miSymbolMenu, _miDryModeStyle, _miAlignHorizontally,
      _miAlignVertically, _miDistributeEvenly, _miAdd, _miRemove, _miGroup,
      _miDesign;
  Gtk::Menu _symbolMenu, _dryModeStyleMenu;
  Gtk::MenuItem _miProperties;
  Gtk::MenuItem _miSaveImage;
  std::vector<Gtk::MenuItem> _miSymbols;
  Gtk::RadioMenuItem _miDMSPrimary, _miDMSSecondary, _miDMSVertical,
      _miDMSHorizontal, _miDMSShadow;

  void inializeContextMenu();
  void initialize();
  void drawAll(const Cairo::RefPtr<Cairo::Context> &cairo);
  void drawFixtures(const Cairo::RefPtr<Cairo::Context> &cairo,
                    const std::vector<theatre::Fixture *> &selection,
                    size_t width, size_t height);
  void onTheatreChanged();
  bool onButtonPress(GdkEventButton *event);
  bool onButtonRelease(GdkEventButton *event);
  bool onMotion(GdkEventMotion *event);
  bool onExpose(const Cairo::RefPtr<Cairo::Context> &context);
  bool onTimeout() {
    Update();
    return true;
  }

  void onFixtureProperties();
  void onSaveImage();
  void onAlignHorizontally();
  void onAlignVertically();
  void onDistributeEvenly();
  void onAddFixtures();
  void onRemoveFixtures();
  void onGroupFixtures();
  void onDesignFixtures();
  void onSetSymbol(theatre::FixtureSymbol::Symbol symbol);
  void onGlobalSelectionChanged();

  void selectFixtures(const theatre::Position &a, const theatre::Position &b);
  void addFixtures(const theatre::Position &a, const theatre::Position &b);
};

}  // namespace glight::gui

#endif
