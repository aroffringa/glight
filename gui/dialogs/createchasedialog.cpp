
#include "createchasedialog.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "theatre/chase.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/presetvalue.h"
#include "theatre/sequence.h"

namespace glight::gui {

CreateChaseDialog::CreateChaseDialog()
    : Dialog("Create chase", true),
      _listFrame("Object list"),
      _list(),
      _newChaseFrame("Chase objects"),
      _addObjectToChaseButton(),
      _clearChaseButton("Clear") {
  set_size_request(600, 400);

  initListPart();
  initNewSequencePart();

  _paned.set_orientation(Gtk::Orientation::VERTICAL);
  _paned.set_start_child(_listFrame);
  _paned.set_end_child(_newChaseFrame);
  get_content_area()->append(_paned);

  add_button("Cancel", Gtk::ResponseType::CANCEL);
  _makeChaseButton = add_button("Make chase", Gtk::ResponseType::OK);
  _makeChaseButton->signal_clicked().connect(
      sigc::mem_fun(*this, &CreateChaseDialog::onCreateChaseButtonClicked),
      false);
  _makeChaseButton->set_sensitive(false);
}

void CreateChaseDialog::initListPart() {
  _list.SignalSelectionChange().connect(
      sigc::mem_fun(*this, &CreateChaseDialog::onSelectedObjectChanged));
  _list.SetDisplayType(ObjectListType::OnlyPresetCollections);

  _listVBox.append(_list);
  _listFrame.set_child(_listVBox);
}

void CreateChaseDialog::initNewSequencePart() {
  _addObjectToChaseButton.set_sensitive(false);
  _addObjectToChaseButton.set_image_from_icon_name("list-add");
  _addObjectToChaseButton.signal_clicked().connect(sigc::mem_fun(
      *this, &CreateChaseDialog::onAddObjectToChaseButtonClicked));
  _newChaseButtonBox.set_orientation(Gtk::Orientation::VERTICAL);
  _newChaseButtonBox.set_homogeneous(true);
  _newChaseButtonBox.append(_addObjectToChaseButton);

  _clearChaseButton.signal_clicked().connect(
      sigc::mem_fun(*this, &CreateChaseDialog::onClearSequenceButtonClicked));
  _newChaseButtonBox.append(_clearChaseButton);

  _newChaseBox.append(_newChaseButtonBox);

  _newChaseListModel = Gtk::ListStore::create(_newChaseListColumns);

  _newChaseListView.set_model(_newChaseListModel);
  _newChaseListView.append_column("Chase object list",
                                  _newChaseListColumns._title);
  _newChaseScrolledWindow.set_child(_newChaseListView);

  _newChaseScrolledWindow.set_policy(Gtk::PolicyType::NEVER,
                                     Gtk::PolicyType::AUTOMATIC);
  _newChaseBox.append(_newChaseScrolledWindow);

  _newChaseFrame.set_child(_newChaseBox);
}

void CreateChaseDialog::onAddObjectToChaseButtonClicked() {
  const system::ObservingPtr<theatre::FolderObject> selectedObj =
      _list.SelectedObject();
  if (selectedObj) {
    theatre::Controllable *object =
        dynamic_cast<theatre::Controllable *>(selectedObj.Get());
    if (object) {
      Gtk::TreeModel::iterator newRow = _newChaseListModel->append();
      std::lock_guard<std::mutex> lock(Instance::Management().Mutex());
      (*newRow)[_newChaseListColumns._title] = object->Name();
      (*newRow)[_newChaseListColumns._controllable] = object;
      _makeChaseButton->set_sensitive(true);
    }
  }
}

void CreateChaseDialog::onClearSequenceButtonClicked() {
  _newChaseListModel->clear();
  _makeChaseButton->set_sensitive(false);
}

void CreateChaseDialog::onCreateChaseButtonClicked() {
  if (!_newChaseListModel->children().empty()) {
    // Determine folder
    theatre::Folder &folder = _list.SelectedFolder();
    theatre::Management &management = Instance::Management();
    std::unique_lock<std::mutex> lock(management.Mutex());

    system::ObservingPtr<theatre::Chase> new_chase = management.AddChasePtr();
    management.AddSourceValue(*new_chase, 0);
    new_chase->SetName(folder.GetAvailableName("Chase"));
    folder.Add(new_chase);

    theatre::Sequence &sequence = new_chase->GetSequence();
    Gtk::TreeModel::Children children = _newChaseListModel->children();
    for (const Gtk::TreeRow &row : children) {
      theatre::Controllable *object = row[_newChaseListColumns._controllable];
      sequence.Add(*object, 0);
    }

    lock.unlock();

    Instance::Events().EmitUpdate();
    _newChaseListModel->clear();
    _makeChaseButton->set_sensitive(false);

    signal_new_chase_(*new_chase);
  }
}

void CreateChaseDialog::onSelectedObjectChanged() {
  if (_delayUpdates.IsFirst()) {
    theatre::FolderObject *selectedObj = _list.SelectedObject().Get();
    theatre::PresetCollection *preset =
        dynamic_cast<theatre::PresetCollection *>(selectedObj);
    if (preset)
      _addObjectToChaseButton.set_sensitive(true);
    else
      _addObjectToChaseButton.set_sensitive(false);
  }
}

}  // namespace glight::gui
