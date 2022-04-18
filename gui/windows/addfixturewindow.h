#ifndef ADD_FIXTURE_WINDOW_H
#define ADD_FIXTURE_WINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/combobox.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/window.h>

class EventTransmitter;
class FixtureType;
class Management;

/**
 * @author Andre Offringa
 */
class AddFixtureWindow : public Gtk::Window {
 public:
  AddFixtureWindow(EventTransmitter *eventHub, Management &management);

 private:
  struct TypeColumns : public Gtk::TreeModelColumnRecord {
    TypeColumns() {
      add(type_);
      add(type_str_);
    }
    Gtk::TreeModelColumn<const FixtureType *> type_;
    Gtk::TreeModelColumn<Glib::ustring> type_str_;
  } type_columns_;

  Gtk::Grid _grid;
  Gtk::Label _typeLabel;
  Glib::RefPtr<Gtk::ListStore> type_model_;
  Gtk::ComboBox _typeCombo;
  Gtk::Label _countLabel;
  Gtk::Entry _countEntry;
  Gtk::Button _decCountButton;
  Gtk::Button _incCountButton;
  Gtk::ButtonBox _buttonBox;
  Gtk::Button _cancelButton;
  Gtk::Button _addButton;

  EventTransmitter &_eventHub;
  Management *_management;

  std::map<std::string, FixtureType> stock_list_;

  void onIncCount();
  void onDecCount();

  void onCancel() { hide(); }
  void onAdd();
};

#endif
