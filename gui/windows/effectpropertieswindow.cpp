#include "effectpropertieswindow.h"

#include <gtkmm/messagedialog.h>

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

  _topBox.append(_titleLabel);

  _addConnectionButton.set_image_from_icon_name("list-add");
  _addConnectionButton.signal_clicked().connect(
      sigc::mem_fun(*this, &EffectPropertiesWindow::onAddConnectionClicked));
  _connectionsButtonBox.set_orientation(Gtk::Orientation::VERTICAL);
  _connectionsButtonBox.append(_addConnectionButton);

  _connectControllablesButton.set_image_from_icon_name("folder");
  _connectControllablesButton.signal_clicked().connect(sigc::mem_fun(
      *this, &EffectPropertiesWindow::onConnectControllableClicked));
  _connectionsButtonBox.append(_connectControllablesButton);

  _removeConnectionButton.set_image_from_icon_name("list-remove");
  _removeConnectionButton.signal_clicked().connect(
      sigc::mem_fun(*this, &EffectPropertiesWindow::onRemoveConnectionClicked));
  _removeConnectionButton.set_sensitive(false);
  _connectionsButtonBox.append(_removeConnectionButton);

  _connectionsBox.append(_connectionsButtonBox);

  _connectionsListModel = Gtk::ListStore::create(_connectionsListColumns);

  _connectionsListView.set_model(_connectionsListModel);
  _connectionsListView.append_column("Connected control",
                                     _connectionsListColumns._title);
  _connectionsListView.get_selection()->signal_changed().connect(sigc::mem_fun(
      *this, &EffectPropertiesWindow::onSelectedConnectionChanged));
  _connectionsScrolledWindow.set_child(_connectionsListView);

  _connectionsScrolledWindow.set_policy(Gtk::PolicyType::NEVER,
                                        Gtk::PolicyType::AUTOMATIC);
  _connectionsScrolledWindow.set_expand(true);
  _connectionsBox.append(_connectionsScrolledWindow);
  _connectionsBox.set_expand(true);
  _connectionsFrame.set_child(_connectionsBox);
  _connectionsFrame.set_expand(true);
  _mainHBox.append(_connectionsFrame);

  _propertiesFrame.set_child(_propertiesBox);
  _propertiesFrame.set_expand(true);
  _mainHBox.append(_propertiesFrame);

  _topBox.append(_mainHBox);

  set_child(_topBox);

  fillConnectionsList();
  _propertySet = theatre::PropertySet::Make(effect);
  _propertiesBox.SetPropertySet(_propertySet.get());
}

theatre::FolderObject& EffectPropertiesWindow::GetObject() { return *_effect; }

void EffectPropertiesWindow::fillConnectionsList() {
  _connectionsListModel->clear();

  std::lock_guard<std::mutex> lock(Instance::Management().Mutex());
  for (size_t index = 0; index != _effect->Connections().size(); ++index) {
    Gtk::TreeModel::iterator iter = _connectionsListModel->append();
    Gtk::TreeModel::Row& row = *iter;
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
  dialog_ = std::make_unique<InputSelectDialog>(true);
  dialog_->signal_response().connect([&](int response) {
    if (response == Gtk::ResponseType::OK) {
      InputSelectDialog& dialog = static_cast<InputSelectDialog&>(*dialog_);
      onInputsSelected({dialog.SelectedSourceValue()});
      const bool stay_open = dialog.StayOpenRequested();
      if (!stay_open) dialog_.reset();
    } else {
      dialog_.reset();
    }
  });
  dialog_->show();
}

void EffectPropertiesWindow::onConnectControllableClicked() {
  dialog_ = std::make_unique<MultiControllableSelectionDialog>();
  dialog_->signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      std::vector<theatre::SourceValue*> sources;
      MultiControllableSelectionDialog& dialog =
          static_cast<MultiControllableSelectionDialog&>(*dialog_);
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
    dialog_.reset();
  });
  dialog_->show();
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
            false, Gtk::MessageType::ERROR);
        dialog.show();
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
