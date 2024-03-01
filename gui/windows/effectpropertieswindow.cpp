#include "effectpropertieswindow.h"

#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "gui/eventtransmitter.h"
#include "gui/instance.h"
#include "gui/dialogs/inputselectdialog.h"
#include "gui/dialogs/multicontrollableselectiondialog.h"
#include "gui/mainwindow/mainwindow.h"

#include "theatre/effect.h"
#include "theatre/management.h"
#include "theatre/sourcevalue.h"

namespace glight::gui {

EffectPropertiesWindow::EffectPropertiesWindow(theatre::Effect& effect)
    : PropertiesWindow(),
      _titleLabel("Effect " + effect.Name() + " (" +
                  EffectTypeToName(effect.GetType()) + ")"),
      _effect(&effect) {
  set_title("glight - " + effect.Name());
  set_size_request(650, 250);

  update_connection_ = Instance::Events().SignalUpdateControllables().connect(
      sigc::mem_fun(*this, &EffectPropertiesWindow::onUpdateControllables));

  _topBox.pack_start(_titleLabel);

  _addConnectionButton.set_image_from_icon_name("list-add");
  _addConnectionButton.signal_clicked().connect(
      sigc::mem_fun(*this, &EffectPropertiesWindow::onAddConnectionClicked));
  _connectionsButtonBox.set_homogeneous(true);
  _connectionsButtonBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
  _connectionsButtonBox.pack_start(_addConnectionButton);

  _connectControllablesButton.set_image_from_icon_name("folder");
  _connectControllablesButton.signal_clicked().connect(sigc::mem_fun(
      *this, &EffectPropertiesWindow::onConnectControllableClicked));
  _connectionsButtonBox.pack_start(_connectControllablesButton);

  _removeConnectionButton.set_image_from_icon_name("list-remove");
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

theatre::FolderObject& EffectPropertiesWindow::GetObject() { return *_effect; }

void EffectPropertiesWindow::fillConnectionsList() {
  _connectionsListModel->clear();

  std::lock_guard<std::mutex> lock(Instance::Management().Mutex());
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
  InputSelectDialog dialog(true);
  bool stay_open;
  do {
    stay_open = false;
    if (dialog.run() == Gtk::RESPONSE_OK) {
      onInputsSelected({dialog.SelectedSourceValue()});
      stay_open = dialog.StayOpenRequested();
    }
  } while (stay_open);
}

void EffectPropertiesWindow::onConnectControllableClicked() {
  MultiControllableSelectionDialog dialog;
  if (dialog.run() == Gtk::RESPONSE_OK) {
    std::vector<theatre::SourceValue*> sources;
    for (theatre::Controllable* controllable : dialog.GetSelection()) {
      for (size_t i = 0; i != controllable->NInputs(); ++i) {
        const theatre::FunctionType input_type = controllable->InputType(i);
        if (IsColor(input_type) ||
            input_type == theatre::FunctionType::Master) {
          glight::theatre::SourceValue* source_value =
              Instance::Management().GetSourceValue(*controllable, i);
          sources.emplace_back(source_value);
        }
      }
    }
    onInputsSelected(sources);
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

void EffectPropertiesWindow::onInputsSelected(
    const std::vector<theatre::SourceValue*>& sources) {
  {
    std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
    for (theatre::SourceValue* source_value : sources) {
      _effect->AddConnection(source_value->GetControllable(),
                             source_value->InputIndex());
      if (Instance::Management().HasCycle()) {
        _effect->RemoveConnection(_effect->Connections().size() - 1);
        lock.unlock();
        Gtk::MessageDialog dialog(
            "Can not add this connection to this effect: "
            "this would create a cycle in the connections.",
            false, Gtk::MESSAGE_ERROR);
        dialog.run();
        break;
      }
    }
  }
  fillConnectionsList();
}

void EffectPropertiesWindow::onUpdateControllables() {
  if (Instance::Management().Contains(*_effect)) {
    fillConnectionsList();
    _propertySet = theatre::PropertySet::Make(*_effect);
    _propertiesBox.SetPropertySet(_propertySet.get());
  } else
    hide();
}

}  // namespace glight::gui
