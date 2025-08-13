#ifndef GUI_FIXTURE_LIST_WINDOW_H_
#define GUI_FIXTURE_LIST_WINDOW_H_

#include <gtkmm/grid.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>

#include "theatre/forwards.h"
#include "theatre/fixturemode.h"

#include "gui/scopedconnection.h"
#include "gui/recursionlock.h"
#include "gui/windows/childwindow.h"

#include <memory>

namespace glight::gui {
class EventTransmitter;
class FixtureSelection;
}  // namespace glight::gui

namespace glight::gui::windows {

class AddFixtureWindow;

/**
 * @author Andre Offringa
 */
class FixtureListWindow : public ChildWindow {
 public:
  FixtureListWindow();
  ~FixtureListWindow();

  void SetLayoutLocked(bool locked) {
    new_button_.set_sensitive(!locked);
    remove_button_.set_sensitive(!locked);
    inc_channel_button_.set_sensitive(!locked);
    dec_channel_button_.set_sensitive(!locked);
    set_channel_button_.set_sensitive(!locked);
    up_button_.set_sensitive(!locked);
    down_button_.set_sensitive(!locked);
    reassign_button_.set_sensitive(!locked);
  }

 private:
  std::vector<system::ObservingPtr<theatre::Fixture>> GetSelection() const;
  void update() { fillFixturesList(); }
  void fillFixturesList();
  void onNewButtonClicked();
  void onRemoveButtonClicked();
  template <int ChannelIncrease, int UniverseIncrease>
  void IncreaseChannelOrUniverse();
  void onSetChannelButtonClicked();
  void updateFixture(const theatre::Fixture *fixture);
  static std::string getChannelString(const theatre::Fixture &fixture);
  void onSelectionChanged();
  void onGlobalSelectionChange();
  void onUpClicked();
  void onDownClicked();
  void onReassignClicked();

  std::unique_ptr<AddFixtureWindow> add_fixture_window_;

  ScopedConnection update_controllables_connection_;
  ScopedConnection global_selection_connection_;
  RecursionLock recursion_lock_;

  Gtk::TreeView fixtures_list_view_;
  Glib::RefPtr<Gtk::ListStore> fixtures_list_model_;
  struct FixturesListColumns : public Gtk::TreeModelColumnRecord {
    FixturesListColumns() {
      add(title_);
      add(type_);
      add(channels_);
      add(universe_);
      add(symbol_);
      add(fixture_);
    }

    Gtk::TreeModelColumn<Glib::ustring> title_, type_;
    Gtk::TreeModelColumn<Glib::ustring> channels_;
    Gtk::TreeModelColumn<unsigned> universe_;
    Gtk::TreeModelColumn<Glib::ustring> symbol_;
    Gtk::TreeModelColumn<system::ObservingPtr<theatre::Fixture>> fixture_;
  } fixtures_list_columns_;
  Gtk::ScrolledWindow fixtures_scrolled_window_;

  Gtk::Grid grid_;

  Gtk::Button new_button_{"New"};
  Gtk::Button remove_button_{"Remove"};
  Gtk::Button inc_channel_button_{"+ C"};
  Gtk::Button dec_channel_button_{"- C"};
  Gtk::Button inc_universe_button_{"+ U"};
  Gtk::Button dec_universe_button_{"- U"};
  Gtk::Button set_channel_button_{"Set..."};
  Gtk::Button up_button_;
  Gtk::Button down_button_;
  Gtk::Button reassign_button_{"Reassign"};
};

}  // namespace glight::gui::windows

#endif
