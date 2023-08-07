#ifndef GUI_FIXTURE_LIST_WINDOW_H_
#define GUI_FIXTURE_LIST_WINDOW_H_

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "../../theatre/forwards.h"
#include "../../theatre/fixturetype.h"

#include "../recursionlock.h"

#include <memory>

namespace glight::gui {

class AddFixtureWindow;
class EventTransmitter;
class FixtureSelection;

/**
 * @author Andre Offringa
 */
class FixtureListWindow : public Gtk::Window {
 public:
  FixtureListWindow(EventTransmitter &eventHub, theatre::Management &management,
                    FixtureSelection &globalSelection);
  ~FixtureListWindow();

 private:
  void update() { fillFixturesList(); }
  void fillFixturesList();
  void onNewButtonClicked();
  void onRemoveButtonClicked();
  void onIncChannelButtonClicked();
  void onDecChannelButtonClicked();
  void onSetChannelButtonClicked();
  // void onMenuItemClicked(StockFixture cl);
  void updateFixture(const theatre::Fixture *fixture);
  static std::string getChannelString(const theatre::Fixture &fixture);
  void onSelectionChanged();
  void onGlobalSelectionChange();
  void onUpClicked();
  void onDownClicked();
  void onReassignClicked();

  EventTransmitter &_eventHub;
  theatre::Management &_management;
  FixtureSelection &_globalSelection;

  std::unique_ptr<AddFixtureWindow> add_fixture_window_;

  sigc::connection _updateControllablesConnection;
  sigc::connection _globalSelectionConnection;
  RecursionLock _recursionLock;

  Gtk::TreeView _fixturesListView;
  Glib::RefPtr<Gtk::ListStore> _fixturesListModel;
  struct FixturesListColumns : public Gtk::TreeModelColumnRecord {
    FixturesListColumns() {
      add(_title);
      add(_type);
      add(_channels);
      add(_symbol);
      add(_fixture);
    }

    Gtk::TreeModelColumn<Glib::ustring> _title, _type, _channels, _symbol;
    Gtk::TreeModelColumn<theatre::Fixture *> _fixture;
  } _fixturesListColumns;
  Gtk::ScrolledWindow _fixturesScrolledWindow;

  Gtk::VBox _mainBox;
  Gtk::ButtonBox _buttonBox;

  Gtk::Button _newButton;
  Gtk::Button _removeButton;
  Gtk::Button _incChannelButton;
  Gtk::Button _decChannelButton;
  Gtk::Button _setChannelButton;
  Gtk::Button _upButton;
  Gtk::Button _downButton;
  Gtk::Button _reassignButton;
};

}  // namespace glight::gui

#endif
