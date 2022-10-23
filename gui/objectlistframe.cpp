#include "objectlistframe.h"

#include <gtkmm/stock.h>

#include "createchasedialog.h"

#include "windows/chasepropertieswindow.h"
#include "windows/effectpropertieswindow.h"
#include "windows/presetcollectionwindow.h"
#include "windows/showwindow.h"
#include "windows/timesequencepropertieswindow.h"

#include "../theatre/chase.h"
#include "../theatre/folder.h"
#include "../theatre/management.h"
#include "../theatre/presetcollection.h"
#include "../theatre/presetvalue.h"
#include "../theatre/sequence.h"
#include "../theatre/timesequence.h"

namespace glight::gui {

using theatre::Effect;
using theatre::EffectType;
using theatre::Folder;
using theatre::FolderObject;

ObjectListFrame::ObjectListFrame(theatre::Management &management,
                                 ShowWindow &parentWindow)
    : _objectListFrame("Object programming"),
      _list(management, parentWindow),
      _newPresetButton("Preset"),
      _newChaseButton("Chase"),
      _newTimeSequenceButton("Sequence"),
      _newEffectButton("Effect"),
      _newFolderButton("Folder"),
      _deletePresetButton(Gtk::Stock::DELETE),
      _management(&management),
      _parentWindow(parentWindow),
      _nameFrame(management, parentWindow) {
  initPresetsPart();

  pack1(_objectListFrame);
  // pack2(_newSequenceFrame);

  show_all_children();
}

void ObjectListFrame::initPresetsPart() {
  _newPresetButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ObjectListFrame::onNewPresetButtonClicked));
  _newPresetButton.set_image_from_icon_name("document-new");
  _presetsButtonBox.pack_start(_newPresetButton, false, false, 5);

  _newChaseButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ObjectListFrame::onNewChaseButtonClicked));
  _newChaseButton.set_image_from_icon_name("document-new");
  _presetsButtonBox.pack_start(_newChaseButton, false, false, 5);

  _newTimeSequenceButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ObjectListFrame::onNewTimeSequenceButtonClicked));
  _newTimeSequenceButton.set_image_from_icon_name("document-new");
  _presetsButtonBox.pack_start(_newTimeSequenceButton, false, false, 5);

  _newEffectButton.set_events(Gdk::BUTTON_PRESS_MASK);
  _newEffectButton.signal_button_press_event().connect(
      sigc::mem_fun(*this, &ObjectListFrame::onNewEffectButtonClicked), false);
  _newEffectButton.set_image_from_icon_name("document-new");
  _presetsButtonBox.pack_start(_newEffectButton);

  _newFolderButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ObjectListFrame::onNewFolderButtonClicked));
  _newFolderButton.set_image_from_icon_name("folder-new");
  _presetsButtonBox.pack_start(_newFolderButton, false, false, 5);

  _deletePresetButton.set_sensitive(false);
  _deletePresetButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ObjectListFrame::onDeletePresetButtonClicked));
  _presetsButtonBox.pack_start(_deletePresetButton, false, false, 5);

  _presetsHBox.pack_start(_presetsButtonBox, false, false);

  _list.SignalSelectionChange().connect(
      sigc::mem_fun(this, &ObjectListFrame::onSelectedObjectChanged));
  _list.SignalObjectActivated().connect(
      sigc::mem_fun(this, &ObjectListFrame::onObjectActivated));
  _list.SetDisplayType(ObjectListType::AllExceptFixtures);
  _list.SetShowTypeColumn(true);
  _presetsHBox.pack_start(_list);

  _presetsVBox.pack_start(_presetsHBox);

  _presetsVBox.pack_start(_nameFrame, false, false, 2);

  _objectListFrame.add(_presetsVBox);
}

void ObjectListFrame::onNewPresetButtonClicked() {
  Folder &parent = _list.SelectedFolder();
  std::unique_lock<std::mutex> lock(_management->Mutex());
  theatre::PresetCollection &presetCollection =
      _management->AddPresetCollection();
  presetCollection.SetName(parent.GetAvailableName("Preset"));
  parent.Add(presetCollection);
  presetCollection.SetFromCurrentSituation(*_management);
  _management->AddSourceValue(presetCollection, 0);
  lock.unlock();

  _parentWindow.EmitUpdate();
  _list.SelectObject(presetCollection);
}

void ObjectListFrame::onNewChaseButtonClicked() {
  CreateChaseDialog dialog(*_management, _parentWindow);
  if (dialog.run() == Gtk::RESPONSE_OK) {
    _list.SelectObject(dialog.CreatedChase());
    onObjectActivated(dialog.CreatedChase());
  }
}

void ObjectListFrame::onNewTimeSequenceButtonClicked() {
  Folder &parent = _list.SelectedFolder();
  std::unique_lock<std::mutex> lock(_management->Mutex());
  theatre::TimeSequence &tSequence = _management->AddTimeSequence();
  tSequence.SetName(parent.GetAvailableName("Seq"));
  parent.Add(tSequence);
  _management->AddSourceValue(tSequence, 0);
  lock.unlock();

  _parentWindow.EmitUpdate();
  _list.SelectObject(tSequence);
  onObjectActivated(tSequence);
}

bool ObjectListFrame::onNewEffectButtonClicked(GdkEventButton *event) {
  if (event->button == 1) {
    _popupEffectMenuItems.clear();
    _popupEffectMenu.reset(new Gtk::Menu());

    std::vector<EffectType> fxtypes = Effect::GetTypes();
    for (EffectType t : fxtypes) {
      std::unique_ptr<Gtk::MenuItem> mi(
          new Gtk::MenuItem(Effect::TypeToName(t)));
      mi->signal_activate().connect(sigc::bind<EffectType>(
          sigc::mem_fun(*this, &ObjectListFrame::onNewEffectMenuClicked), t));
      _popupEffectMenu->append(*mi);
      _popupEffectMenuItems.emplace_back(std::move(mi));
    }

    _popupEffectMenu->show_all_children();
    _popupEffectMenu->popup(event->button, event->time);
    return true;
  }
  return false;
}

void ObjectListFrame::onNewEffectMenuClicked(
    enum theatre::EffectType effectType) {
  std::unique_ptr<Effect> effect(Effect::Make(effectType));
  Folder &parent = _list.SelectedFolder();
  effect->SetName(parent.GetAvailableName(Effect::TypeToName(effectType)));
  Effect *added = &_management->AddEffect(std::move(effect), parent);
  for (size_t i = 0; i != added->NInputs(); ++i)
    _management->AddSourceValue(*added, i);
  _parentWindow.EmitUpdate();
  _list.SelectObject(*added);
  onObjectActivated(*added);
}

void ObjectListFrame::onNewFolderButtonClicked() {
  Folder &parent = _list.SelectedFolder();
  std::unique_lock<std::mutex> lock(_management->Mutex());
  Folder &folder =
      _management->AddFolder(parent, parent.GetAvailableName("Folder"));
  lock.unlock();

  _parentWindow.EmitUpdate();
  _list.SelectObject(folder);
}

void ObjectListFrame::onDeletePresetButtonClicked() {
  FolderObject *selectedObj = _list.SelectedObject();
  if (selectedObj && selectedObj != &_management->RootFolder()) {
    {
      std::lock_guard<std::mutex> lock(_management->Mutex());
      _management->RemoveObject(*selectedObj);
    }
    _parentWindow.EmitUpdate();
  }
}

void ObjectListFrame::onSelectedObjectChanged() {
  if (_delayUpdates.IsFirst()) {
    FolderObject *selectedObj = _list.SelectedObject();
    if (selectedObj) {
      _nameFrame.SetNamedObject(*selectedObj);
      _deletePresetButton.set_sensitive(true);
    } else {
      _nameFrame.SetNoNamedObject();
      _deletePresetButton.set_sensitive(false);
    }
  }
}

void ObjectListFrame::onObjectActivated(FolderObject &object) {
  PropertiesWindow *window = _windowList.GetOpenWindow(object);
  if (window) {
    window->present();
  } else {
    theatre::PresetCollection *presetCollection =
        dynamic_cast<theatre::PresetCollection *>(&object);
    if (presetCollection) {
      std::unique_ptr<PresetCollectionWindow> newWindow(
          new PresetCollectionWindow(*presetCollection, *_management,
                                     _parentWindow));
      newWindow->present();
      _windowList.Add(std::move(newWindow));
    }
    theatre::Chase *chase = dynamic_cast<theatre::Chase *>(&object);
    if (chase) {
      std::unique_ptr<ChasePropertiesWindow> newWindow(
          new ChasePropertiesWindow(*chase, *_management, _parentWindow));
      newWindow->present();
      _windowList.Add(std::move(newWindow));
    }
    theatre::TimeSequence *timeSequence =
        dynamic_cast<theatre::TimeSequence *>(&object);
    if (timeSequence) {
      std::unique_ptr<TimeSequencePropertiesWindow> newWindow(
          new TimeSequencePropertiesWindow(*timeSequence, *_management,
                                           _parentWindow));
      newWindow->present();
      _windowList.Add(std::move(newWindow));
    }
    theatre::Effect *effect = dynamic_cast<theatre::Effect *>(&object);
    if (effect) {
      std::unique_ptr<EffectPropertiesWindow> newWindow(
          new EffectPropertiesWindow(*effect, *_management, _parentWindow));
      newWindow->present();
      _windowList.Add(std::move(newWindow));
    }
  }
}

}  // namespace glight::gui
