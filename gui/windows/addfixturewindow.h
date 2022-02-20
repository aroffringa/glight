#ifndef ADD_FIXTURE_WINDOW_H
#define ADD_FIXTURE_WINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

/**
        @author Andre Offringa
*/
class AddFixtureWindow : public Gtk::Window {
 public:
  AddFixtureWindow(class EventTransmitter *eventHub,
                   class Management &management);

 private:
  Gtk::Grid _grid;
  Gtk::Label _typeLabel;
  Gtk::ComboBoxText _typeCombo;
  Gtk::Label _countLabel;
  Gtk::Entry _countEntry;
  Gtk::Button _decCountButton, _incCountButton;
  Gtk::ButtonBox _buttonBox;
  Gtk::Button _cancelButton, _addButton;

  EventTransmitter &_eventHub;
  Management *_management;

  void onIncCount();
  void onDecCount();

  void onCancel() { hide(); }
  void onAdd();
};

#endif
