#include "actions.h"

#include "gui/eventtransmitter.h"
#include "gui/functions.h"
#include "gui/instance.h"

#include "gui/dialogs/createchasedialog.h"

#include "gui/windows/chasepropertieswindow.h"
#include "gui/windows/effectpropertieswindow.h"
#include "gui/windows/groupwindow.h"
#include "gui/windows/presetcollectionwindow.h"
#include "gui/windows/propertieswindow.h"
#include "gui/windows/timesequencepropertieswindow.h"

#include "theatre/chase.h"
#include "theatre/fixturegroup.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/presetvalue.h"
#include "theatre/sequence.h"
#include "theatre/timesequence.h"

namespace glight::gui::mainwindow {

using theatre::Effect;
using theatre::EffectType;
using theatre::Folder;
using theatre::FolderObject;
using theatre::Management;

PropertiesWindow &OpenPropertiesWindow(
    ObjectWindowList<PropertiesWindow> &property_windows, FolderObject &object,
    Gtk::Window &parent) {
  PropertiesWindow *window = property_windows.GetOpenWindow(object);
  if (!window) {
    using theatre::Chase;
    using theatre::Effect;
    using theatre::FixtureGroup;
    using theatre::PresetCollection;
    using theatre::TimeSequence;
    std::unique_ptr<PropertiesWindow> new_window;
    if (Chase *chase = dynamic_cast<Chase *>(&object); chase) {
      new_window = std::make_unique<ChasePropertiesWindow>(*chase);
    } else if (theatre::Effect *effect =
                   dynamic_cast<theatre::Effect *>(&object);
               effect) {
      new_window = std::make_unique<EffectPropertiesWindow>(*effect);
    } else if (theatre::FixtureGroup *group =
                   dynamic_cast<theatre::FixtureGroup *>(&object);
               group) {
      new_window = std::make_unique<windows::GroupWindow>(*group);
    } else if (PresetCollection *presetCollection =
                   dynamic_cast<PresetCollection *>(&object);
               presetCollection) {
      new_window = std::make_unique<PresetCollectionWindow>(*presetCollection);
    } else if (TimeSequence *timeSequence =
                   dynamic_cast<TimeSequence *>(&object);
               timeSequence) {
      new_window =
          std::make_unique<TimeSequencePropertiesWindow>(*timeSequence);
    }
    if (new_window) {
      window = new_window.get();
      property_windows.Add(std::move(new_window));
    }
  }
  if (window) window->present();
  return *window;
}

void NewEmptyPreset(ObjectBrowser &browser,
                    ObjectWindowList<PropertiesWindow> &property_windows,
                    Gtk::Window &parent) {
  Folder &parent_folder = browser.SelectedFolder();
  Management &management = Instance::Management();
  std::unique_lock<std::mutex> lock(management.Mutex());
  ObservingPtr<theatre::PresetCollection> preset_collection =
      management.AddPresetCollectionPtr();
  preset_collection->SetName(parent_folder.GetAvailableName("Preset"));
  parent_folder.Add(preset_collection);
  management.AddSourceValue(*preset_collection, 0);
  lock.unlock();

  Instance::Events().EmitUpdate();
  browser.SelectObject(*preset_collection);
  AssignFader(*preset_collection);
  OpenPropertiesWindow(property_windows, *preset_collection, parent);
}

void NewPresetFromCurrent(ObjectBrowser &browser) {
  Folder &parent = browser.SelectedFolder();
  Management &management = Instance::Management();
  std::unique_lock<std::mutex> lock(management.Mutex());
  ObservingPtr<theatre::PresetCollection> preset_collection =
      management.AddPresetCollectionPtr();
  preset_collection->SetName(parent.GetAvailableName("Preset"));
  parent.Add(preset_collection);
  preset_collection->SetFromCurrentSituation(management);
  management.AddSourceValue(*preset_collection, 0);
  lock.unlock();

  Instance::Events().EmitUpdate();
  browser.SelectObject(*preset_collection);
  AssignFader(*preset_collection);
}

void NewPresetFromFixtures(
    theatre::Folder &parent_folder,
    const std::set<ObservingPtr<theatre::Fixture>, std::less<>> &fixtures) {
  Management &management = Instance::Management();
  std::unique_lock<std::mutex> lock(management.Mutex());
  ObservingPtr<theatre::PresetCollection> preset_collection =
      management.AddPresetCollectionPtr();
  preset_collection->SetName(parent_folder.GetAvailableName("Preset"));
  parent_folder.Add(preset_collection);
  preset_collection->SetFromCurrentFixtures(management, fixtures);
  management.AddSourceValue(*preset_collection, 0);
  lock.unlock();

  AssignFader(*preset_collection);
  Instance::Events().EmitUpdate();
}

void NewChase(ObjectBrowser &browser,
              ObjectWindowList<PropertiesWindow> &property_windows,
              Gtk::Window &parent) {
  CreateChaseDialog dialog;
  if (dialog.run() == Gtk::RESPONSE_OK) {
    theatre::Chase &new_chase = dialog.CreatedChase();
    browser.SelectObject(new_chase);
    AssignFader(new_chase);
    OpenPropertiesWindow(property_windows, new_chase, parent);
  }
}

void NewTimeSequence(ObjectBrowser &browser,
                     ObjectWindowList<PropertiesWindow> &property_windows,
                     Gtk::Window &parent) {
  Management &management = Instance::Management();
  std::unique_lock<std::mutex> lock(management.Mutex());
  Folder &parent_folder = browser.SelectedFolder();
  ObservingPtr<theatre::TimeSequence> time_sequence =
      management.AddTimeSequencePtr();
  time_sequence->SetName(parent_folder.GetAvailableName("Seq"));
  parent_folder.Add(time_sequence);
  management.AddSourceValue(*time_sequence, 0);
  lock.unlock();

  Instance::Events().EmitUpdate();
  browser.SelectObject(*time_sequence);
  AssignFader(*time_sequence);
  OpenPropertiesWindow(property_windows, *time_sequence, parent);
}

void NewEffect(theatre::EffectType effect_type, ObjectBrowser &browser,
               ObjectWindowList<PropertiesWindow> &property_windows,
               Gtk::Window &parent) {
  std::unique_ptr<Effect> effect(Effect::Make(effect_type));
  Management &management = Instance::Management();
  std::unique_lock<std::mutex> lock(management.Mutex());
  Folder &parent_folder = browser.SelectedFolder();
  effect->SetName(
      parent_folder.GetAvailableName(EffectTypeToName(effect_type)));
  ObservingPtr<Effect> added =
      management.AddEffectPtr(std::move(effect), parent_folder);
  for (size_t i = 0; i != added->NInputs(); ++i)
    management.AddSourceValue(*added, i);
  lock.unlock();
  Instance::Events().EmitUpdate();
  browser.SelectObject(*added);
  AssignFader(*added);
  OpenPropertiesWindow(property_windows, *added, parent);
}

void NewFolder(ObjectBrowser &browser) {
  Folder &parent_folder = browser.SelectedFolder();
  Management &management = Instance::Management();
  std::unique_lock<std::mutex> lock(management.Mutex());
  Folder &folder = management.AddFolder(
      parent_folder, parent_folder.GetAvailableName("Folder"));
  lock.unlock();

  Instance::Events().EmitUpdate();
  browser.SelectObject(folder);
}

void DeleteObject(ObjectBrowser &browser) {
  ObservingPtr<FolderObject> object = browser.SelectedObject();
  Management &management = Instance::Management();
  std::unique_lock<std::mutex> lock(management.Mutex());
  if (object && object != &management.RootFolder()) {
    management.RemoveObject(*object);
    lock.unlock();
    Instance::Events().EmitUpdate();
  }
}

}  // namespace glight::gui::mainwindow
