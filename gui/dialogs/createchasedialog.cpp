
#include "createchasedialog.h"

#include <gtkmm/stock.h>

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
      _clearChaseButton("Clear"),
      _newChase(nullptr) {
  set_size_request(600, 400);

  initListPart();
  initNewSequencePart();

  _paned.set_orientation(Gtk::ORIENTATION_VERTICAL);
  _paned.pack1(_listFrame);
  _paned.pack2(_newChaseFrame);
  get_content_area()->pack_start(_paned);

  add_button("Cancel", Gtk::RESPONSE_CANCEL);
  _makeChaseButton = add_button("Make chase", Gtk::RESPONSE_OK);
  _makeChaseButton->signal_clicked().connect(
      sigc::mem_fun(*this, &CreateChaseDialog::onCreateChaseButtonClicked));
  _makeChaseButton->set_sensitive(false);

  show_all_children();
}

void CreateChaseDialog::initListPart() {
  _list.SignalSelectionChange().connect(
      sigc::mem_fun(this, &CreateChaseDialog::onSelectedObjectChanged));
  _list.SetDisplayType(ObjectListType::OnlyPresetCollections);

  _listVBox.pack_start(_list);
  _listFrame.add(_listVBox);
}

void CreateChaseDialog::initNewSequencePart() {
  _addObjectToChaseButton.set_sensitive(false);
  _addObjectToChaseButton.set_image_from_icon_name("list-add");
  _addObjectToChaseButton.signal_clicked().connect(sigc::mem_fun(
      *this, &CreateChaseDialog::onAddObjectToChaseButtonClicked));
  _newChaseButtonBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
  _newChaseButtonBox.set_homogeneous(true);
  _newChaseButtonBox.pack_start(_addObjectToChaseButton);

  _clearChaseButton.signal_clicked().connect(
      sigc::mem_fun(*this, &CreateChaseDialog::onClearSequenceButtonClicked));
  _newChaseButtonBox.pack_start(_clearChaseButton);

  _newChaseBox.pack_start(_newChaseButtonBox, false, false, 5);

  _newChaseListModel = Gtk::ListStore::create(_newChaseListColumns);

  _newChaseListView.set_model(_newChaseListModel);
  _newChaseListView.append_column("Chase object list",
                                  _newChaseListColumns._title);
  _newChaseScrolledWindow.add(_newChaseListView);

  _newChaseScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  _newChaseBox.pack_start(_newChaseScrolledWindow);

  _newChaseFrame.add(_newChaseBox);
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

    _newChase = management.AddChase().GetObserver<theatre::Chase>();
    management.AddSourceValue(*_newChase, 0);
    _newChase->SetName(folder.GetAvailableName("Chase"));
    folder.Add(_newChase);

    theatre::Sequence &sequence = _newChase->GetSequence();
    Gtk::TreeModel::Children children = _newChaseListModel->children();
    for (const Gtk::TreeRow &row : children) {
      theatre::Controllable *object = row[_newChaseListColumns._controllable];
      sequence.Add(*object, 0);
    }

    lock.unlock();

    Instance::Events().EmitUpdate();
    _newChaseListModel->clear();
    _makeChaseButton->set_sensitive(false);
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
