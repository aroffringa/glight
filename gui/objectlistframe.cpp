#include "objectlistframe.h"

#include <gtkmm/stock.h>

#include "createchasedialog.h"

#include "windows/chasepropertieswindow.h"
#include "windows/effectpropertieswindow.h"
#include "windows/groupwindow.h"
#include "windows/presetcollectionwindow.h"
#include "windows/showwindow.h"
#include "windows/timesequencepropertieswindow.h"

#include "../theatre/chase.h"
#include "../theatre/fixturegroup.h"
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
      _deletePresetButton("Delete"),
      _management(&management),
      _parentWindow(parentWindow),
      _nameFrame(management, parentWindow) {
  set_orientation(Gtk::ORIENTATION_VERTICAL);
  initPresetsPart();

  pack1(_objectListFrame);
  // pack2(_newSequenceFrame);

  show_all_children();
}

void ObjectListFrame::initPresetsPart() {
  _newPresetButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ObjectListFrame::onNewPresetButtonClicked));
  _newPresetButton.set_image_from_icon_name("document-new");
  _presetsButtonBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
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
  _deletePresetButton.set_image_from_icon_name("edit-delete");
  _deletePresetButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ObjectListFrame::onDeletePresetButtonClicked));
  _presetsButtonBox.pack_start(_deletePresetButton, false, false, 5);

  _presetsHBox.pack_start(_presetsButtonBox, false, false);

  _list.SignalSelectionChange().connect(
      sigc::mem_fun(this, &ObjectListFrame::onSelectedObjectChanged));
  _list.SignalObjectActivated().connect(
      [&](glight::theatre::FolderObject &object) {
        ObjectListFrame::OpenPropertiesWindow(object);
      });
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
    OpenPropertiesWindow(dialog.CreatedChase());
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
  OpenPropertiesWindow(tSequence);
}

bool ObjectListFrame::onNewEffectButtonClicked(GdkEventButton *event) {
  if (event->button == 1) {
    _popupEffectMenuItems.clear();
    _popupEffectMenu = std::make_unique<Gtk::Menu>();

    std::vector<EffectType> fxtypes = theatre::GetEffectTypes();
    for (EffectType t : fxtypes) {
      std::unique_ptr<Gtk::MenuItem> mi =
          std::make_unique<Gtk::MenuItem>(EffectTypeToName(t));
      mi->signal_activate().connect(sigc::bind<EffectType>(
          sigc::mem_fun(*this, &ObjectListFrame::onNewEffectMenuClicked), t));
      _popupEffectMenu->append(*mi);
      _popupEffectMenuItems.emplace_back(std::move(mi));
    }

    _popupEffectMenu->show_all_children();
    _popupEffectMenu->popup_at_widget(
        &_newEffectButton, Gdk::Gravity::GRAVITY_SOUTH_WEST,
        Gdk::Gravity::GRAVITY_NORTH_WEST,
        reinterpret_cast<const GdkEvent *>(event));
    return true;
  }
  return false;
}

void ObjectListFrame::onNewEffectMenuClicked(
    enum theatre::EffectType effectType) {
  std::unique_ptr<Effect> effect(Effect::Make(effectType));
  Folder &parent = _list.SelectedFolder();
  effect->SetName(parent.GetAvailableName(EffectTypeToName(effectType)));
  Effect *added = &_management->AddEffect(std::move(effect), parent);
  for (size_t i = 0; i != added->NInputs(); ++i)
    _management->AddSourceValue(*added, i);
  _parentWindow.EmitUpdate();
  _list.SelectObject(*added);
  OpenPropertiesWindow(*added);
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

PropertiesWindow &ObjectListFrame::OpenPropertiesWindow(FolderObject &object) {
  PropertiesWindow *window = _windowList.GetOpenWindow(object);
  if (window) {
    window->present();
  } else {
    using theatre::Chase;
    using theatre::Effect;
    using theatre::FixtureGroup;
    using theatre::PresetCollection;
    using theatre::TimeSequence;
    std::unique_ptr<PropertiesWindow> new_window;
    if (Chase *chase = dynamic_cast<Chase *>(&object); chase) {
      new_window = std::make_unique<ChasePropertiesWindow>(*chase, *_management,
                                                           _parentWindow);
    } else if (theatre::Effect *effect =
                   dynamic_cast<theatre::Effect *>(&object);
               effect) {
      new_window = std::make_unique<EffectPropertiesWindow>(
          *effect, *_management, _parentWindow);
    } else if (theatre::FixtureGroup *group =
                   dynamic_cast<theatre::FixtureGroup *>(&object);
               group) {
      new_window = std::make_unique<windows::GroupWindow>(*group, *_management,
                                                          _parentWindow);
    } else if (PresetCollection *presetCollection =
                   dynamic_cast<PresetCollection *>(&object);
               presetCollection) {
      new_window = std::make_unique<PresetCollectionWindow>(
          *presetCollection, *_management, _parentWindow);
    } else if (TimeSequence *timeSequence =
                   dynamic_cast<TimeSequence *>(&object);
               timeSequence) {
      new_window = std::make_unique<TimeSequencePropertiesWindow>(
          *timeSequence, *_management, _parentWindow);
    }
    new_window->present();
    window = new_window.get();
    _windowList.Add(std::move(new_window));
  }
  return *window;
}

}  // namespace glight::gui
