#ifndef GUI_INPUT_SELECT_WIDGET_H_
#define GUI_INPUT_SELECT_WIDGET_H_

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>

#include "objectbrowser.h"

#include "../../theatre/controllable.h"
#include "../../theatre/forwards.h"

namespace glight::gui {

class EventTransmitter;

class InputSelectWidget : public Gtk::VBox {
 public:
  static const size_t NO_INPUT_SELECTED = std::numeric_limits<size_t>::max();

  InputSelectWidget()
      : _browser(),
        _inputLabel("Input:"),
        _selectedObject(nullptr),
        _selectedInput(NO_INPUT_SELECTED) {
    _browser.SetDisplayType(ObjectListType::All);
    _browser.SignalSelectionChange().connect(
        [&]() { onBrowserSelectionChange(); });
    pack_start(_browser, true, true);

    _listModel = Gtk::ListStore::create(_listColumns);

    _inputBox.pack_start(_inputLabel, false, false, 5);

    _inputCombo.set_size_request(200, 0);
    _inputCombo.set_model(_listModel);
    _inputCombo.pack_start(_listColumns._title);
    _inputCombo.signal_changed().connect([&]() { onComboSelectionChange(); });
    _inputBox.pack_start(_inputCombo, false, false, 5);

    pack_end(_inputBox, false, false);

    show_all_children();
  }

  sigc::signal<void()> &SignalSelectionChange() {
    return _signalSelectionChange;
  }

  theatre::Controllable *SelectedObject() const { return _selectedObject; }
  size_t SelectedInput() const { return _selectedInput; }
  bool HasInputSelected() const { return _selectedInput != NO_INPUT_SELECTED; }

 private:
  ObjectBrowser _browser;
  Gtk::HBox _inputBox;
  Gtk::Label _inputLabel;
  Gtk::ComboBox _inputCombo;

  theatre::Controllable *_selectedObject;
  size_t _selectedInput;

  sigc::signal<void()> _signalSelectionChange;
  RecursionLock _recursionLock;

  Glib::RefPtr<Gtk::ListStore> _listModel;
  struct ListColumns : public Gtk::TreeModelColumnRecord {
    ListColumns() {
      add(_title);
      add(_inputIndex);
    }

    Gtk::TreeModelColumn<Glib::ustring> _title;
    Gtk::TreeModelColumn<size_t> _inputIndex;
  } _listColumns;

  void fillCombo() {
    RecursionLock::Token token(_recursionLock);
    _listModel->clear();

    if (_selectedObject) {
      for (size_t input = 0; input != _selectedObject->NInputs(); ++input) {
        Gtk::TreeModel::iterator iter = _listModel->append();
        Gtk::TreeModel::Row row = *iter;
        row[_listColumns._title] = std::to_string(input + 1) + ". " +
                                   ToString(_selectedObject->InputType(input));
        row[_listColumns._inputIndex] = input;
      }
    }
  }

  void onBrowserSelectionChange() {
    theatre::Controllable *controllable =
        dynamic_cast<theatre::Controllable *>(_browser.SelectedObject().Get());
    if (controllable != _selectedObject) {
      bool selectionChanged = (_selectedInput != NO_INPUT_SELECTED);
      _selectedObject = controllable;
      fillCombo();
      if (controllable && controllable->NInputs() == 1) {
        RecursionLock::Token token(_recursionLock);
        _inputCombo.set_active(0);
        _selectedInput = 0;
        selectionChanged = true;
      } else {
        _selectedInput = NO_INPUT_SELECTED;
      }

      if (selectionChanged) _signalSelectionChange.emit();
    }
  }

  void onComboSelectionChange() {
    if (_recursionLock.IsFirst()) {
      Gtk::TreeModel::iterator selected = _inputCombo.get_active();
      size_t newInput;
      if (selected)
        newInput = (*selected)[_listColumns._inputIndex];
      else
        newInput = NO_INPUT_SELECTED;
      if (newInput != _selectedInput) {
        _selectedInput = newInput;
        _signalSelectionChange.emit();
      }
    }
  }
};

}  // namespace glight::gui

#endif
