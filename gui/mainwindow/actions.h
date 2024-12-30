#ifndef GLIGHT_GUI_MAINWINDOW_ACTIONS_H_
#define GLIGHT_GUI_MAINWINDOW_ACTIONS_H_

#include <set>

#include "theatre/effecttype.h"
#include "theatre/fixture.h"
#include "theatre/folderobject.h"

#include "gui/components/objectbrowser.h"
#include "gui/mainwindow/objectwindowlist.h"
#include "gui/windows/propertieswindow.h"

namespace glight::gui::mainwindow {

PropertiesWindow& OpenPropertiesWindow(
    ObjectWindowList<PropertiesWindow>& property_windows,
    theatre::FolderObject& object, Gtk::Window& parent);

void NewEmptyPreset(ObjectBrowser& browser,
                    ObjectWindowList<PropertiesWindow>& property_windows,
                    Gtk::Window& parent);
void NewPresetFromCurrent(ObjectBrowser& browser);
void NewPresetFromFixtures(
    theatre::Folder& parent_folder,
    const std::set<system::ObservingPtr<theatre::Fixture>, std::less<>>&
        fixtures);
void NewChase(ObjectBrowser& browser,
              ObjectWindowList<PropertiesWindow>& property_windows,
              Gtk::Window& parent);
void NewTimeSequence(ObjectBrowser& browser,
                     ObjectWindowList<PropertiesWindow>& property_windows,
                     Gtk::Window& parent);
void NewFolder(ObjectBrowser& browser);
void NewEffect(theatre::EffectType effect_type, ObjectBrowser& browser,
               ObjectWindowList<PropertiesWindow>& property_windows,
               Gtk::Window& parent);
void DeleteObject(ObjectBrowser& browser);

}  // namespace glight::gui::mainwindow

#endif
