#include "multicontrollableselectiondialog.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "theatre/management.h"
#include "theatre/presetcollection.h"

#include <gtkmm/messagedialog.h>

namespace glight::gui {

using theatre::Controllable;

MultiControllableSelectionDialog::MultiControllableSelectionDialog() {
  update_connection_ =
      Instance::Events().SignalUpdateControllables().connect(sigc::mem_fun(
          *this, &MultiControllableSelectionDialog::OnUpdateControllables));

  set_title("glight - select controllables");

  object_browser_.SignalSelectionChange().connect(sigc::mem_fun(
      *this, &MultiControllableSelectionDialog::OnObjectSelectionChanged));
  object_browser_.SetDisplayType(ObjectListType::All);
  object_browser_.SetAllowMultiSelection(true);
  box_.pack_start(object_browser_);

  add_button_.set_image_from_icon_name("go-next");
  add_button_.set_sensitive(false);
  add_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &MultiControllableSelectionDialog::OnAdd));
  button_box_.pack_start(add_button_, false, false, 4);

  remove_button_.set_image_from_icon_name("go-previous");
  remove_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &MultiControllableSelectionDialog::OnRemove));
  button_box_.pack_start(remove_button_, false, false, 4);

  button_box_.set_valign(Gtk::ALIGN_CENTER);
  box_.pack_start(button_box_, false, false, 4);

  selection_store_ = Gtk::ListStore::create(selection_columns_);

  selection_view_.set_model(selection_store_);
  selection_view_.append_column("Controllable", selection_columns_.name_);
  selection_view_.get_selection()->signal_changed().connect(sigc::mem_fun(
      *this, &MultiControllableSelectionDialog::OnViewSelectionChanged));
  selection_scrolled_window_.add(selection_view_);

  selection_scrolled_window_.set_size_request(200, 200);
  selection_scrolled_window_.set_policy(Gtk::POLICY_NEVER,
                                        Gtk::POLICY_AUTOMATIC);
  grid_.attach(selection_scrolled_window_, 0, 0, 2, 1);
  selection_scrolled_window_.set_hexpand(true);
  selection_scrolled_window_.set_vexpand(true);

  box_.pack_start(grid_, true, true, 4);
  get_content_area()->pack_start(box_);
  show_all_children();
  add_button("Select", Gtk::RESPONSE_OK);
  add_button("Cancel", Gtk::RESPONSE_CANCEL);
}

bool MultiControllableSelectionDialog::IsSelected(
    const theatre::Controllable& controllable) const {
  for (const Gtk::TreeRow& row : selection_store_->children()) {
    if (row[selection_columns_.controllable_] == &controllable) return true;
  }
  return false;
}

std::vector<theatre::Controllable*>
MultiControllableSelectionDialog::GetSelection() const {
  std::vector<theatre::Controllable*> result;
  result.reserve(selection_store_->children().size());
  for (const Gtk::TreeRow& row : selection_store_->children()) {
    result.emplace_back(row[selection_columns_.controllable_]);
  }
  return result;
}

void MultiControllableSelectionDialog::SetSelection(
    const std::vector<Controllable*>& selection) {
  RecursionLock::Token token(recursion_lock_);
  selection_store_->clear();
  for (Controllable* controllable : selection) {
    Gtk::TreeModel::iterator iter = selection_store_->append();
    const Gtk::TreeModel::Row& row = *iter;
    row[selection_columns_.name_] = controllable->Name();
    row[selection_columns_.controllable_] = controllable;
  }
  token.Release();
}

void MultiControllableSelectionDialog::OnObjectSelectionChanged() {
  add_button_.set_sensitive(!object_browser_.Selection().empty());
}

void MultiControllableSelectionDialog::OnViewSelectionChanged() {
  const Gtk::TreeModel::iterator iter =
      selection_view_.get_selection()->get_selected();
  remove_button_.set_sensitive(static_cast<bool>(iter));
}

void MultiControllableSelectionDialog::OnAdd() {
  const std::vector<theatre::FolderObject*> objects =
      object_browser_.Selection();
  for (theatre::FolderObject* object : objects) {
    Controllable* controllable = dynamic_cast<Controllable*>(object);
    if (controllable && !IsSelected(*controllable)) {
      Gtk::TreeModel::iterator iter = selection_store_->append();
      const Gtk::TreeModel::Row& row = *iter;
      row[selection_columns_.name_] = controllable->Name();
      row[selection_columns_.controllable_] = controllable;
    }
  }
}

void MultiControllableSelectionDialog::OnRemove() {
  Gtk::TreeModel::iterator selection_iter =
      selection_view_.get_selection()->get_selected();
  if (selection_iter) {
    selection_store_->erase(selection_iter);
  }
}

void MultiControllableSelectionDialog::OnUpdateControllables() {
  theatre::Management& management = Instance::Management();
  std::vector<Controllable*> new_selection;
  for (const Gtk::TreeRow& row : selection_store_->children()) {
    if (management.Contains(*row[selection_columns_.controllable_])) {
      new_selection.emplace_back(row[selection_columns_.controllable_]);
    }
  }
  SetSelection(new_selection);
}

}  // namespace glight::gui
