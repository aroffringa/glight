#include "propertiesbox.h"

#include "durationinput.h"
#include "transitiontypebox.h"

namespace glight::gui {

using theatre::Property;
using theatre::PropertyType;

PropertiesBox::PropertiesBox()
    : _typeLabel("No object selected"), _applyButton("Apply") {
  set_orientation(Gtk::Orientation::VERTICAL);
  append(_typeLabel);

  _grid.set_column_homogeneous(false);
  _grid.set_column_spacing(0);
  append(_grid);

  _propertiesButtonBox.set_homogeneous(true);

  _applyButton.set_sensitive(false);
  _applyButton.signal_clicked().connect(
      sigc::mem_fun(*this, &PropertiesBox::onApplyClicked));
  _propertiesButtonBox.append(_applyButton);

  append(_propertiesButtonBox);
}

void PropertiesBox::Clear() {
  _typeLabel.set_text("No object selected");
  _propertySet = nullptr;
  _rows.clear();
  _applyButton.set_sensitive(false);
}

void PropertiesBox::fillProperties() {
  _rows.clear();
  _typeLabel.set_text(
      _propertySet->Object().Name() + " (" +
      theatre::EffectTypeToName(
          static_cast<theatre::Effect &>(_propertySet->Object()).GetType()) +
      ")");
  for (theatre::Property &property : *_propertySet) {
    size_t rowIndex = _rows.size();
    _rows.emplace_back();
    PropertyRow &row = _rows.back();

    switch (property.GetType()) {
      case PropertyType::Boolean: {
        std::unique_ptr<Gtk::CheckButton> button =
            std::make_unique<Gtk::CheckButton>(property.Description());
        button->set_active(_propertySet->GetBool(property));
        row._widgets.emplace_back(std::move(button));
        _grid.attach(*row._widgets.back(), 0, rowIndex, 2, 1);
      } break;
      case PropertyType::Choice: {
        row._widgets.emplace_back(
            std::make_unique<Gtk::Box>(Gtk::Orientation::VERTICAL));
        Gtk::Box *box = static_cast<Gtk::Box *>(row._widgets.back().get());
        row._widgets.emplace_back(
            std::make_unique<Gtk::Label>(property.Description()));
        box->append(*row._widgets.back());
        std::string value = _propertySet->GetChoice(property);
        Gtk::CheckButton *first_button;
        for (size_t i = 0; i != property.OptionCount(); ++i) {
          std::unique_ptr<Gtk::CheckButton> button =
              std::make_unique<Gtk::CheckButton>(property.OptionDescription(i));
          if (i == 0)
            first_button = button.get();
          else
            button->set_group(*first_button);
          button->set_active(property.OptionName(i) == value);
          row._widgets.emplace_back(std::move(button));
          box->append(*row._widgets.back());
        }
        _grid.attach(*box, 0, rowIndex, 2, 1);
      } break;
      case PropertyType::ControlValue: {
        std::string entryText = std::to_string(
            round(1000.0 * _propertySet->GetControlValue(property) /
                  theatre::ControlValue::MaxUInt()) /
            10.0);

        row._widgets.emplace_back(
            std::make_unique<Gtk::Label>(property.Description()));
        _grid.attach(*row._widgets.back(), 0, rowIndex, 1, 1);

        std::unique_ptr<Gtk::Entry> entry = std::make_unique<Gtk::Entry>();
        entry->set_text(entryText);
        _grid.attach(*entry, 1, rowIndex, 1, 1);
        row._widgets.emplace_back(std::move(entry));
      } break;
      case PropertyType::Duration: {
        double duration = _propertySet->GetDuration(property);

        row._widgets.emplace_back(
            std::make_unique<DurationInput>(property.Description(), duration));
        _grid.attach(*row._widgets.back(), 0, rowIndex, 2, 1);
      } break;
      case PropertyType::Integer: {
        std::string entryText =
            std::to_string(_propertySet->GetInteger(property));

        row._widgets.emplace_back(
            std::make_unique<Gtk::Label>(property.Description()));
        _grid.attach(*row._widgets.back(), 0, rowIndex, 1, 1);

        std::unique_ptr<Gtk::Entry> entry = std::make_unique<Gtk::Entry>();
        entry->set_text(entryText);
        _grid.attach(*entry, 1, rowIndex, 1, 1);
        row._widgets.emplace_back(std::move(entry));
      } break;
      case PropertyType::Transition: {
        const theatre::Transition transition =
            _propertySet->GetTransition(property);
        row._widgets.emplace_back(
            std::make_unique<Gtk::Box>(Gtk::Orientation::VERTICAL));
        Gtk::Box *box = static_cast<Gtk::Box *>(row._widgets.back().get());
        row._widgets.emplace_back(std::make_unique<DurationInput>(
            property.Description(), transition.LengthInMs()));
        box->append(*row._widgets.back());
        row._widgets.emplace_back(
            std::make_unique<TransitionTypeBox>(transition.Type()));
        box->append(*row._widgets.back());
        _grid.attach(*box, 0, rowIndex, 2, 1);
      } break;
    }
  }
  _applyButton.set_sensitive(true);
}

void PropertiesBox::onApplyClicked() {
  auto rowIter = _rows.begin();
  for (theatre::Property &property : *_propertySet) {
    switch (property.GetType()) {
      case PropertyType::Boolean: {
        bool value = static_cast<Gtk::CheckButton *>(rowIter->_widgets[0].get())
                         ->get_active();
        _propertySet->SetBool(property, value);
      } break;
      case PropertyType::Choice: {
        for (size_t i = 0; i != property.OptionCount(); ++i) {
          if (static_cast<Gtk::CheckButton *>(rowIter->_widgets[2 + i].get())
                  ->get_active()) {
            _propertySet->SetChoice(property, property.OptionName(i));
            break;
          }
        }
      } break;
      case PropertyType::ControlValue: {
        std::string entryText =
            static_cast<Gtk::Entry *>(rowIter->_widgets[1].get())->get_text();
        _propertySet->SetControlValue(
            property,
            static_cast<unsigned>(std::atof(entryText.c_str()) *
                                  theatre::ControlValue::MaxUInt() / 100.0));
      } break;
      case PropertyType::Duration: {
        double value =
            static_cast<DurationInput *>(rowIter->_widgets[0].get())->Value();
        _propertySet->SetDuration(property, value);
      } break;
      case PropertyType::Integer: {
        std::string entryText =
            static_cast<Gtk::Entry *>(rowIter->_widgets[1].get())->get_text();
        _propertySet->SetInteger(property, std::atoi(entryText.c_str()));
      } break;
      case PropertyType::Transition: {
        const DurationInput *di =
            static_cast<DurationInput *>(rowIter->_widgets[1].get());
        const TransitionTypeBox *tb =
            static_cast<TransitionTypeBox *>(rowIter->_widgets[2].get());
        _propertySet->SetTransition(
            property, theatre::Transition(di->Value(), tb->Get()));
      }
    }
    ++rowIter;
  }
}

}  // namespace glight::gui
