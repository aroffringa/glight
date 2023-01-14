#include "effectpropertieswindow.h"
#include "showwindow.h"

#include "../dialogs/inputselectdialog.h"

#include "../../theatre/effect.h"
#include "../../theatre/management.h"
#include "../../theatre/sourcevalue.h"

#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

namespace glight::gui {

EffectPropertiesWindow::EffectPropertiesWindow(theatre::Effect &effect,
                                               theatre::Management &management,
                                               ShowWindow &parentWindow)
    : PropertiesWindow(),

      _titleLabel("Effect " + effect.Name() + " (" +
                  EffectTypeToName(effect.GetType()) + ")"),
      _connectionsFrame("Connections"),
      _propertiesFrame("Properties"),
      _addConnectionButton(Gtk::Stock::ADD),
      _removeConnectionButton(Gtk::Stock::REMOVE),

      _effect(&effect),
      _management(&management),
      _parentWindow(parentWindow) {
  set_title("glight - " + effect.Name());
  set_size_request(650, 250);

  parentWindow.SignalUpdateControllables().connect(
      sigc::mem_fun(*this, &EffectPropertiesWindow::onUpdateControllables));

  _topBox.pack_start(_titleLabel);

  _addConnectionButton.signal_pressed().connect(
      sigc::mem_fun(*this, &EffectPropertiesWindow::onAddConnectionClicked));
  _connectionsButtonBox.pack_start(_addConnectionButton);

  _removeConnectionButton.signal_clicked().connect(
      sigc::mem_fun(*this, &EffectPropertiesWindow::onRemoveConnectionClicked));
  _removeConnectionButton.set_sensitive(false);
  _connectionsButtonBox.pack_start(_removeConnectionButton);

  _connectionsBox.pack_start(_connectionsButtonBox, false, false, 5);

  _connectionsListModel = Gtk::ListStore::create(_connectionsListColumns);

  _connectionsListView.set_model(_connectionsListModel);
  _connectionsListView.append_column("Connected control",
                                     _connectionsListColumns._title);
  _connectionsListView.get_selection()->signal_changed().connect(sigc::mem_fun(
      *this, &EffectPropertiesWindow::onSelectedConnectionChanged));
  _connectionsScrolledWindow.add(_connectionsListView);

  _connectionsScrolledWindow.set_policy(Gtk::POLICY_NEVER,
                                        Gtk::POLICY_AUTOMATIC);
  _connectionsBox.pack_start(_connectionsScrolledWindow, true, true);
  _connectionsFrame.add(_connectionsBox);
  _mainHBox.pack_start(_connectionsFrame);

  _propertiesFrame.add(_propertiesBox);

  _mainHBox.pack_start(_propertiesFrame, true, true);

  _topBox.pack_start(_mainHBox, true, true);

  add(_topBox);

  fillConnectionsList();
  _propertySet = theatre::PropertySet::Make(effect);
  _propertiesBox.SetPropertySet(_propertySet.get());

  show_all_children();
}

theatre::FolderObject &EffectPropertiesWindow::GetObject() { return *_effect; }

void EffectPropertiesWindow::fillConnectionsList() {
  _connectionsListModel->clear();

  std::lock_guard<std::mutex> lock(_management->Mutex());
  for (size_t index = 0; index != _effect->Connections().size(); ++index) {
    Gtk::TreeModel::iterator iter = _connectionsListModel->append();
    const Gtk::TreeModel::Row& row = *iter;
    row[_connectionsListColumns._title] =
        _effect->Connections()[index].first->InputName(
            _effect->Connections()[index].second);
    row[_connectionsListColumns._index] = index;
    row[_connectionsListColumns._inputIndex] =
        _effect->Connections()[index].second;
  }
}

void EffectPropertiesWindow::onSelectedConnectionChanged() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _connectionsListView.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  _removeConnectionButton.set_sensitive(bool(selected));
}

void EffectPropertiesWindow::onAddConnectionClicked() {
  InputSelectDialog dialog(*_management, _parentWindow);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    onInputSelected(dialog.SelectedInputPreset());
  }
}

void EffectPropertiesWindow::onRemoveConnectionClicked() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _connectionsListView.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected)
    _effect->RemoveConnection((*selected)[_connectionsListColumns._index]);
  fillConnectionsList();
}

void EffectPropertiesWindow::onInputSelected(
    theatre::SourceValue *sourceValue) {
  std::unique_lock<std::mutex> lock(_management->Mutex());
  _effect->AddConnection(sourceValue->GetControllable(),
                         sourceValue->InputIndex());
  if (_management->HasCycle()) {
    _effect->RemoveConnection(_effect->Connections().size() - 1);
    lock.unlock();
    Gtk::MessageDialog dialog(
        "Can not add this connection to this effect: "
        "this would create a cycle in the connections.",
        false, Gtk::MESSAGE_ERROR);
    dialog.run();
  } else {
    lock.unlock();
    fillConnectionsList();
  }
}

void EffectPropertiesWindow::onUpdateControllables() {
  if (_management->Contains(*_effect)) {
    fillConnectionsList();
    _propertySet = theatre::PropertySet::Make(*_effect);
    _propertiesBox.SetPropertySet(_propertySet.get());
  } else
    hide();
}

}  // namespace glight::gui
