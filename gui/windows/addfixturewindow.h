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

namespace glight::gui {
class EventTransmitter;
}

namespace glight::gui::windows {

/**
 * @author Andre Offringa
 */
class AddFixtureWindow : public Gtk::Window {
 public:
  AddFixtureWindow(EventTransmitter *eventHub, theatre::Management &management);

 private:
  void updateFilters();
  void fillStock();
  void fillFromProject();

  struct TypeColumns : public Gtk::TreeModelColumnRecord {
    TypeColumns() {
      add(type_);
      add(type_str_);
    }
    Gtk::TreeModelColumn<const theatre::FixtureType *> type_;
    Gtk::TreeModelColumn<Glib::ustring> type_str_;
  } type_columns_;

  Gtk::Grid _grid;
  Gtk::HBox stock_or_project_box_;
  Gtk::RadioButton stock_button_{"Stock"};
  Gtk::RadioButton project_button_{"Project"};
  Gtk::Label _typeLabel{"Type:"};
  Glib::RefPtr<Gtk::ListStore> type_model_;
  Gtk::ComboBox _typeCombo;
  Gtk::Label _countLabel{"Count:"};
  Gtk::Entry _countEntry;
  Gtk::Button _decCountButton{"-"};
  Gtk::Button _incCountButton{"+"};
  Gtk::Frame filters_frame_{"Filters"};
  Gtk::VBox filters_box_;
  Gtk::CheckButton auto_master_cb_{"Auto master channel"};
  Gtk::CheckButton rgb_cb_{"RGB colorspace"};
  Gtk::CheckButton monochrome_cb_{"Monochrome"};
  Gtk::CheckButton temperature_cb_{"Temperature to RGB"};
  Gtk::Box _buttonBox;
  Gtk::Button _cancelButton{"Cancel"};
  Gtk::Button _addButton{"Add"};

  EventTransmitter &_eventHub;
  theatre::Management *_management;

  const std::map<std::string, theatre::FixtureType> stock_list_;

  void onStockProjectToggled();

  void onIncCount();
  void onDecCount();

  void onCancel() { hide(); }
  void onAdd();
};

}  // namespace glight::gui::windows

#endif
