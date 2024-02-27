#include "groupwindow.h"

#include <algorithm>

#include "gui/instance.h"

#include "theatre/fixture.h"
#include "theatre/fixturegroup.h"
#include "theatre/management.h"

namespace glight::gui::windows {

using theatre::Fixture;
using theatre::NamedObject;

GroupWindow::GroupWindow(theatre::FixtureGroup& group)
    : group_(group), fixture_list_(), reorder_widget_() {
  box_.pack_start(fixture_list_);
  add_button_.signal_clicked().connect([&]() { Append(); });
  box_.pack_start(add_button_, false, false);
  add_button_.set_valign(Gtk::Align::ALIGN_CENTER);
  add_button_.set_image_from_icon_name("go-next");
  box_.pack_start(reorder_widget_);
  reorder_widget_.SignalChanged().connect([&]() { StoreGroup(); });
  group_delete_connection_ = group.SignalDelete().connect([&]() { hide(); });
  add(box_);
  show_all_children();
  LoadGroup();
}

theatre::FolderObject& GroupWindow::GetObject() { return group_; }

void GroupWindow::Append() {
  std::vector<Fixture*> fixtures = fixture_list_.Selection();
  std::vector<NamedObject*> objects = reorder_widget_.GetList();
  std::sort(objects.begin(), objects.end());
  for (Fixture* fixture : fixtures) {
    if (!std::binary_search(objects.begin(), objects.end(), fixture)) {
      reorder_widget_.Append(*fixture);
    }
  }
}

void GroupWindow::StoreGroup() {
  std::unique_lock lock(Instance::Management().Mutex());
  group_.Clear();
  std::vector<NamedObject*> objects = reorder_widget_.GetList();
  for (NamedObject* object : objects) {
    group_.Insert(static_cast<Fixture&>(*object));
  }
}

void GroupWindow::LoadGroup() { reorder_widget_.SetList(group_.Fixtures()); }

}  // namespace glight::gui::windows
