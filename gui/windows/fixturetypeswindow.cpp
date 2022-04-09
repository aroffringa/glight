#include "fixturetypeswindow.h"

#include "../eventtransmitter.h"
#include "../fixtureselection.h"

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "../../theatre/fixture.h"
#include "../../theatre/fixturecontrol.h"
#include "../../theatre/folder.h"
#include "../../theatre/management.h"
#include "../../theatre/theatre.h"

FixtureTypesWindow::FixtureTypesWindow(EventTransmitter *eventHub,
                                       Management &management)
    : event_hub_(eventHub),
      management_(&management),
      class_label_("Class:"),
      functions_label_("Functions:"),
      add_function_button_("+"),
      remove_function_button_("-"),
      new_button_("New"),
      remove_button_("Remove"),
      save_button_("Save") {
  set_title("Glight - fixture types");
  set_size_request(200, 400);

  change_management_connection_ = event_hub_->SignalChangeManagement().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onChangeManagement));
  update_controllables_connection_ =
      event_hub_->SignalUpdateControllables().connect(
          [&]() { FixtureTypesWindow::update(); });

  // Left part
  list_model_ = Gtk::ListStore::create(list_columns_);

  list_view_.get_selection()->signal_changed().connect(
      [&]() { onSelectionChanged(); });
  list_view_.set_model(list_model_);
  list_view_.append_column("Name", list_columns_.name_);
  list_view_.append_column("Used", list_columns_.in_use_);
  list_view_.append_column("Functions", list_columns_.functions_);
  fillList();
  scrolled_window_.add(list_view_);

  scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  left_box_.pack_start(scrolled_window_);

  main_grid_.attach(left_box_, 0, 0, 1, 1);
  left_box_.set_hexpand(true);
  left_box_.set_vexpand(true);

  // Right part
  right_grid_.attach(class_label_, 0, 0);
  const std::vector<FixtureClass> classes = FixtureType::GetClassList();
  for (FixtureClass c : classes) class_combo_.append(FixtureType::ClassName(c));
  right_grid_.attach(class_combo_, 1, 0);
  right_grid_.attach(functions_label_, 0, 1, 2, 1);

  functions_model_ = Gtk::ListStore::create(functions_columns_);

  functions_view_.get_selection()->signal_changed().connect(
      [&]() { onSelectedFunctionChanged(); });
  functions_view_.set_model(functions_model_);
  functions_view_.append_column("DMX", functions_columns_.dmx_offset_);
  functions_view_.append_column("16 bit?", functions_columns_.is_16_bit_);
  functions_view_.append_column("Type", functions_columns_.function_type_);
  functions_view_.set_vexpand(true);
  right_grid_.attach(functions_view_, 0, 2, 2, 1);

  add_function_button_.signal_clicked().connect([&]() { onAddFunction(); });
  functions_button_box_.pack_start(add_function_button_);
  remove_function_button_.signal_clicked().connect(
      [&]() { onRemoveFunction(); });
  functions_button_box_.pack_end(remove_function_button_);
  right_grid_.attach(functions_button_box_, 1, 3, 1, 1);

  main_grid_.attach(right_grid_, 1, 0, 1, 1);
  right_grid_.set_hexpand(true);
  right_grid_.set_vexpand(true);

  // Buttons at bottom
  new_button_.set_image_from_icon_name("document-new");
  new_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onNewButtonClicked), false);
  button_box_.pack_start(new_button_);

  remove_button_.set_image_from_icon_name("edit-delete");
  remove_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onRemoveButtonClicked));
  button_box_.pack_start(remove_button_);

  save_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onSaveButtonClicked));
  button_box_.pack_start(save_button_);

  main_grid_.attach(button_box_, 0, 1, 2, 1);
  button_box_.set_hexpand(true);
  button_box_.set_vexpand(false);

  add(main_grid_);
  main_grid_.show_all();
}

FixtureTypesWindow::~FixtureTypesWindow() {
  change_management_connection_.disconnect();
  update_controllables_connection_.disconnect();
}

void FixtureTypesWindow::fillList() {
  list_model_->clear();

  std::lock_guard<std::mutex> lock(management_->Mutex());
  const std::vector<std::unique_ptr<FixtureType>> &types =
      management_->GetTheatre().FixtureTypes();
  for (const std::unique_ptr<FixtureType> &type : types) {
    Gtk::TreeModel::iterator iter = list_model_->append();
    Gtk::TreeModel::Row row = *iter;
    row[list_columns_.fixture_type_] = type.get();
    row[list_columns_.name_] = type->Name();
    row[list_columns_.functions_] = type->Name();
    row[list_columns_.in_use_] = management_->GetTheatre().IsUsed(*type);
  }
}

void FixtureTypesWindow::onNewButtonClicked() {}

void FixtureTypesWindow::onRemoveButtonClicked() {
  FixtureType *type = getSelected();
  if (type) {
    management_->RemoveFixtureType(*type);
    event_hub_->EmitUpdate();
  }
}

void FixtureTypesWindow::onSaveButtonClicked() {
  FixtureType *type = getSelected();
  if (type) {
    if (management_->GetTheatre().IsUsed(*type)) {
      Gtk::MessageDialog dialog(*this,
                                "A used type can not be changed: first remove "
                                "all fixtures of this type");
      dialog.run();
    } else {
    }
  }
}

FixtureType *FixtureTypesWindow::getSelected() {
  Glib::RefPtr<Gtk::TreeSelection> selection = list_view_.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected)
    return (*selected)[list_columns_.fixture_type_];
  else
    return nullptr;
}

void FixtureTypesWindow::onSelectionChanged() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    const FixtureType *type = getSelected();
    remove_button_.set_sensitive(type != nullptr);
    save_button_.set_sensitive(type != nullptr);
    right_grid_.set_sensitive(type != nullptr);
    if (type) {
      class_combo_.set_active_text(
          FixtureType::ClassName(type->GetFixtureClass()));
      const std::vector<FixtureTypeFunction> &functions = type->Functions();
      functions_model_->clear();
      for (size_t i = 0; i != functions.size(); ++i) {
        Gtk::TreeModel::iterator iter = functions_model_->append();
        Gtk::TreeModel::Row row = *iter;
        const FixtureTypeFunction &f = functions[i];
        row[functions_columns_.dmx_offset_] = f.dmxOffset;
        row[functions_columns_.is_16_bit_] = f.is16Bit;
        row[functions_columns_.function_type_] =
            FunctionTypeDescription(f.type);
      }
    }
  }
}

void FixtureTypesWindow::onAddFunction() {
  size_t dmx_offset = 0;
  if (!functions_model_->children().empty()) {
    Gtk::TreeModel::Row row = *functions_model_->children().rbegin();
    ;
    if (row[functions_columns_.is_16_bit_])
      dmx_offset = row[functions_columns_.dmx_offset_] + 2;
    else
      dmx_offset = row[functions_columns_.dmx_offset_] + 1;
  }

  Gtk::TreeModel::iterator iter = functions_model_->append();
  Gtk::TreeModel::Row row = *iter;
  row[functions_columns_.dmx_offset_] = dmx_offset;
  row[functions_columns_.is_16_bit_] = false;
  row[functions_columns_.function_type_] =
      FunctionTypeDescription(FunctionType::White);
}

void FixtureTypesWindow::onRemoveFunction() {
  Glib::RefPtr<Gtk::TreeSelection> selection = functions_view_.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    functions_model_->erase(selected);
  }
}

void FixtureTypesWindow::onSelectedFunctionChanged() {}
