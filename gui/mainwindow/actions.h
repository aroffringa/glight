#ifndef GLIGHT_GUI_MAINWINDOW_ACTIONS_H_
#define GLIGHT_GUI_MAINWINDOW_ACTIONS_H_

#include <set>

#include "theatre/effecttype.h"
#include "theatre/fixture.h"
#include "theatre/folderobject.h"

#include "gui/components/objectbrowser.h"
#include "gui/mainwindow/windowlist.h"
#include "gui/windows/propertieswindow.h"

namespace glight::gui::mainwindow {

PropertiesWindow& OpenPropertiesWindow(
    WindowList<PropertiesWindow>& property_windows,
    theatre::FolderObject& object, Gtk::Window& parent);

void NewEmptyPreset(ObjectBrowser& browser,
                    WindowList<PropertiesWindow>& property_windows,
                    Gtk::Window& parent);
void NewPresetFromCurrent(ObjectBrowser& browser);
void NewPresetFromFixtures(theatre::Folder& parent_folder,
                           const std::set<theatre::Fixture*>& fixtures);
void NewChase(ObjectBrowser& browser,
              WindowList<PropertiesWindow>& property_windows,
              Gtk::Window& parent);
void NewTimeSequence(ObjectBrowser& browser,
                     WindowList<PropertiesWindow>& property_windows,
                     Gtk::Window& parent);
void NewFolder(ObjectBrowser& browser);
void NewEffect(theatre::EffectType effect_type, ObjectBrowser& browser,
               WindowList<PropertiesWindow>& property_windows,
               Gtk::Window& parent);
void DeleteObject(ObjectBrowser& browser);

}  // namespace glight::gui::mainwindow

#endif
