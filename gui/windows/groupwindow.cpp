#include "groupwindow.h"

#include <algorithm>

#include "gui/instance.h"

#include "theatre/fixture.h"
#include "theatre/fixturegroup.h"
#include "theatre/management.h"

namespace glight::gui::windows {

using system::ObservingPtr;
using theatre::Fixture;
using theatre::NamedObject;

GroupWindow::GroupWindow(theatre::FixtureGroup& group)
    : group_(group), fixture_list_(), reorder_widget_() {
  box_.append(fixture_list_);
  add_button_.signal_clicked().connect([&]() { Append(); });
  box_.append(add_button_);
  add_button_.set_valign(Gtk::Align::CENTER);
  add_button_.set_image_from_icon_name("go-next");
  box_.append(reorder_widget_);
  reorder_widget_.SignalChanged().connect([&]() { StoreGroup(); });
  group_delete_connection_ = group.SignalDelete().connect([&]() { hide(); });
  set_child(box_);
  LoadGroup();
}

theatre::FolderObject& GroupWindow::GetObject() { return group_; }

void GroupWindow::Append() {
  std::vector<ObservingPtr<theatre::Fixture>> fixtures =
      fixture_list_.Selection();
  std::vector<ObservingPtr<NamedObject>> objects = reorder_widget_.GetList();
  std::sort(objects.begin(), objects.end());
  for (const ObservingPtr<theatre::Fixture>& fixture : fixtures) {
    if (!std::binary_search(objects.begin(), objects.end(), fixture.Get())) {
      reorder_widget_.Append(fixture);
    }
  }
}

void GroupWindow::StoreGroup() {
  std::unique_lock lock(Instance::Management().Mutex());
  group_.Clear();
  std::vector<ObservingPtr<NamedObject>> objects = reorder_widget_.GetList();
  for (ObservingPtr<NamedObject> object : objects) {
    group_.Insert(static_cast<ObservingPtr<Fixture>>(object));
  }
}

void GroupWindow::LoadGroup() { reorder_widget_.SetList(group_.Fixtures()); }

}  // namespace glight::gui::windows
