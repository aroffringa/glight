#ifndef GUI_TRANSITION_TYPE_BOX_H_
#define GUI_TRANSITION_TYPE_BOX_H_

#include "../../theatre/transition.h"

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>

namespace glight::gui {

class TransitionTypeBox : public Gtk::Box {
 public:
  TransitionTypeBox(
      theatre::TransitionType value = theatre::TransitionType::Fade) {
    append(label_);
    label_.show();

    model_ = Gtk::ListStore::create(columns_);
    const std::vector<theatre::TransitionType> types =
        theatre::GetTransitionTypes();
    for (theatre::TransitionType type : types) {
      Gtk::TreeModel::iterator iter = model_->append();
      Gtk::TreeModel::Row& row = *iter;
      row[columns_.description_] = GetDescription(type);
      row[columns_.value_] = type;
    }

    combo_.set_model(model_);
    combo_.pack_start(columns_.description_);

    append(combo_);
    combo_.signal_changed().connect([&]() {
      const int index = combo_.get_active_row_number();
      if (index >= 0) {
        const theatre::TransitionType type =
            theatre::GetTransitionTypes()[index];
        value_ = type;
        signal_changed_(type);
      }
    });
    combo_.show();

    Set(value);
  }

  sigc::signal<void(theatre::TransitionType)>& SignalChanged() {
    return signal_changed_;
  }

  void Set(theatre::TransitionType type) {
    const std::vector<theatre::TransitionType> types =
        theatre::GetTransitionTypes();
    const size_t index =
        std::find(types.begin(), types.end(), type) - types.begin();
    combo_.set_active(index);
    value_ = type;
  }

  theatre::TransitionType Get() const { return value_; }

 private:
  Gtk::Label label_{"Type:"};
  Gtk::ComboBox combo_;
  struct Columns : public Gtk::TreeModelColumnRecord {
    Columns() {
      add(description_);
      add(value_);
    }

    Gtk::TreeModelColumn<Glib::ustring> description_;
    Gtk::TreeModelColumn<theatre::TransitionType> value_;
  } columns_;
  Glib::RefPtr<Gtk::ListStore> model_;
  sigc::signal<void(theatre::TransitionType)> signal_changed_;
  theatre::TransitionType value_;
};

}  // namespace glight::gui

#endif
