#ifndef GUI_ADD_FIXTURE_WINDOW_H_
#define GUI_ADD_FIXTURE_WINDOW_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/entry.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/window.h>

#include "theatre/forwards.h"
#include "system/trackableptr.h"

namespace glight::gui {
class EventTransmitter;
}

namespace glight::gui::windows {

/**
 * @author Andre Offringa
 */
class AddFixtureWindow : public Gtk::Window {
 public:
  AddFixtureWindow();

 private:
  void updateModes();
  void updateFilters();
  void fillStock();
  void fillFromProject();
  system::ObservingPtr<theatre::FixtureType> GetSelectedType(
      system::TrackablePtr<theatre::FixtureType>& stock_type);

  struct TypeColumns : public Gtk::TreeModelColumnRecord {
    TypeColumns() {
      add(type_);
      add(type_str_);
      add(stock_fixture_);
    }
    Gtk::TreeModelColumn<system::ObservingPtr<theatre::FixtureType>> type_;
    Gtk::TreeModelColumn<Glib::ustring> type_str_;
    Gtk::TreeModelColumn<theatre::StockFixture> stock_fixture_;
  } type_columns_;

  struct ModeColumns : public Gtk::TreeModelColumnRecord {
    ModeColumns() {
      add(mode_str_);
      add(mode_index_);
    }
    Gtk::TreeModelColumn<Glib::ustring> mode_str_;
    Gtk::TreeModelColumn<size_t> mode_index_;
  } mode_columns_;

  Gtk::Grid grid_;
  Gtk::HBox stock_or_project_box_;
  Gtk::RadioButton stock_button_{"Stock"};
  Gtk::RadioButton project_button_{"Project"};

  Gtk::Label type_label_{"Type:"};
  Glib::RefPtr<Gtk::ListStore> type_model_;
  Gtk::ComboBox type_combo_;

  Gtk::Label channel_mode_label_{"Channel mode:"};
  Glib::RefPtr<Gtk::ListStore> channel_mode_model_;
  Gtk::ComboBox channel_mode_combo_;

  Gtk::Label count_label_{"Count:"};
  Gtk::Entry count_entry_;
  Gtk::Button decrease_count_button_{"-"};
  Gtk::Button increase_count_button_{"+"};
  Gtk::Frame filters_frame_{"Filters"};
  Gtk::VBox filters_box_;
  Gtk::CheckButton auto_master_cb_{"Auto master channel"};
  Gtk::CheckButton rgb_cb_{"RGB colorspace"};
  Gtk::CheckButton monochrome_cb_{"Monochrome"};
  Gtk::CheckButton temperature_cb_{"Temperature to RGB"};
  Gtk::Box button_box_;
  Gtk::Button cancel_button_{"Cancel"};
  Gtk::Button add_button_{"Add"};

  std::map<std::string, theatre::StockFixture> stock_list_;

  void onStockProjectToggled();

  void onIncCount();
  void onDecCount();

  void onCancel() { hide(); }
  void onAdd();
};

}  // namespace glight::gui::windows

#endif
