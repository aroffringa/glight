#ifndef WINDOWS_GROUP_WINDOW_H_
#define WINDOWS_GROUP_WINDOW_H_

#include <gtkmm/box.h>

#include "propertieswindow.h"

#include "gui/connectionmanager.h"
#include "gui/components/fixturelist.h"
#include "gui/components/reorderwidget.h"

namespace glight::gui::windows {

class GroupWindow final : public PropertiesWindow {
 public:
  GroupWindow(theatre::FixtureGroup &group);

  theatre::FolderObject &GetObject() override;

 private:
  void Append();
  void StoreGroup();
  void LoadGroup();

  theatre::FixtureGroup &group_;
  Gtk::HBox box_;
  components::FixtureList fixture_list_;
  Gtk::Button add_button_;
  components::ReorderWidget reorder_widget_;
  ConnectionManager connections_;
};

}  // namespace glight::gui::windows

#endif
