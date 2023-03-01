#include "sceneselect.h"

#include "../eventtransmitter.h"

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "../../theatre/scenes/scene.h"

#include "../../theatre/management.h"

namespace glight::gui::dialogs {

SceneSelect::SceneSelect(theatre::Management &management,
                         EventTransmitter &event_hub)
    : management_(management), event_hub_(event_hub) {
  set_title("Glight - select scene");
  set_size_request(200, 400);

  update_controllables_connection_ =
      event_hub.SignalUpdateControllables().connect(
          [&]() { SceneSelect::FillScenesList(); });

  model_ = Gtk::ListStore::create(columns_);

  view_.set_model(model_);
  view_.append_column("Scene", columns_.title_);
  view_.append_column("Items", columns_.items_);
  FillScenesList();
  scrolled_window_.add(view_);

  scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  get_content_area()->pack_start(scrolled_window_);
  scrolled_window_.show_all();

  select_button_ = add_button("Select", Gtk::RESPONSE_OK);
  select_button_->set_sensitive(false);
  add_button("Cancel", Gtk::RESPONSE_CANCEL);

  view_.get_selection()->signal_changed().connect(
      [&]() { OnSelectionChanged(); });
}

SceneSelect::~SceneSelect() { update_controllables_connection_.disconnect(); }

void SceneSelect::FillScenesList() {
  model_->clear();

  std::lock_guard<std::mutex> lock(management_.Mutex());
  const std::vector<std::unique_ptr<theatre::Controllable>> &controllables =
      management_.Controllables();
  for (const std::unique_ptr<theatre::Controllable> &controllable :
       controllables) {
    if (theatre::Scene *scene =
            dynamic_cast<theatre::Scene *>(controllable.get());
        scene) {
      Gtk::TreeModel::iterator iter = model_->append();
      const Gtk::TreeModel::Row &row = *iter;
      row[columns_.title_] = scene->Name();
      row[columns_.items_] = scene->SceneItems().size();
      row[columns_.scene_] = scene;
    }
  }
}

void SceneSelect::SetSelection(theatre::Scene &scene) {
  Glib::RefPtr<Gtk::TreeSelection> selection = view_.get_selection();
  selection->unselect_all();

  for (const Gtk::TreeRow &row : model_->children()) {
    if ((*row)[columns_.scene_] == &scene) {
      selection->select(row);
      break;
    }
  }
}

theatre::Scene *SceneSelect::GetSelection() {
  Glib::RefPtr<Gtk::TreeSelection> selection = view_.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    return (*selected)[columns_.scene_];
  } else {
    return nullptr;
  }
}

void SceneSelect::OnSelectionChanged() {
  select_button_->set_sensitive(GetSelection() != nullptr);
}

}  // namespace glight::gui::dialogs
