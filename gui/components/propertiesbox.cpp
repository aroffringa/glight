#include "propertiesbox.h"

#include "durationinput.h"

#include <gtkmm/stock.h>

PropertiesBox::PropertiesBox() :
	_typeLabel("No object selected"),
	_applyButton(Gtk::Stock::APPLY)
{
	pack_start(_typeLabel);
	
	_grid.set_column_homogeneous(false);
	_grid.set_column_spacing(0);
	pack_start(_grid, true, true);
	
	_applyButton.set_sensitive(false);
	_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &PropertiesBox::onApplyClicked));
	_propertiesButtonBox.pack_start(_applyButton);
	
	pack_end(_propertiesButtonBox);
	
	show_all_children();
}

void PropertiesBox::Clear()
{
	_typeLabel.set_text("No object selected");
	_propertySet = nullptr;
	_rows.clear();
	_applyButton.set_sensitive(false);
}

void PropertiesBox::fillProperties()
{
	_rows.clear();
	_typeLabel.set_text(_propertySet->Object().Name() + " (" +
		Effect::TypeToName(static_cast<Effect&>(_propertySet->Object()).GetType()) + ")");
	for(Property& property : *_propertySet)
	{
		size_t rowIndex = _rows.size();
		_rows.emplace_back();
		PropertyRow& row = _rows.back();
		
		switch(property.GetType())
		{
		case Property::Boolean: {
			std::unique_ptr<Gtk::CheckButton> button(new Gtk::CheckButton(property.Description()));
			button->set_active(_propertySet->GetBool(property));
			row._widgets.emplace_back(std::move(button));
			_grid.attach(*row._widgets.back(), 0, rowIndex, 2, 1);
			} break;
		case Property::ControlValue: {
			std::string entryText =
				std::to_string(round(1000.0*_propertySet->GetControlValue(property)/ControlValue::MaxUInt())/10.0);
				
			row._widgets.emplace_back(new Gtk::Label(property.Description()));
			_grid.attach(*row._widgets.back(), 0, rowIndex, 1, 1);
			
			Gtk::Entry* entry = new Gtk::Entry();
			row._widgets.emplace_back(entry);
			entry->set_text(entryText);
			_grid.attach(*entry, 1, rowIndex, 1, 1);
			} break;
		case Property::Duration: {
			double duration = _propertySet->GetDuration(property);
				
			row._widgets.emplace_back(new DurationInput(property.Description(), duration));
			_grid.attach(*row._widgets.back(), 0, rowIndex, 2, 1);
			} break;
		case Property::Integer: {
			std::string entryText = std::to_string(_propertySet->GetInteger(property));
				
			row._widgets.emplace_back(new Gtk::Label(property.Description()));
			_grid.attach(*row._widgets.back(), 0, rowIndex, 1, 1);
			
			Gtk::Entry* entry = new Gtk::Entry();
			row._widgets.emplace_back(entry);
			entry->set_text(entryText);
			_grid.attach(*entry, 1, rowIndex, 1, 1);
			} break;
		}
	}
	_grid.show_all_children();
	_applyButton.set_sensitive(true);
}

void PropertiesBox::onApplyClicked()
{
	auto rowIter = _rows.begin();
	for(Property& property : *_propertySet)
	{
		switch(property.GetType())
		{
		case Property::Boolean: {
			bool value = static_cast<Gtk::CheckButton*>(rowIter->_widgets[0].get())->get_active();
			_propertySet->SetBool(property, value);
			} break;
		case Property::ControlValue: {
			std::string entryText = static_cast<Gtk::Entry*>(rowIter->_widgets[1].get())->get_text();
			_propertySet->SetControlValue(property, unsigned(std::atof(entryText.c_str())*ControlValue::MaxUInt()/100.0));
			} break;
		case Property::Duration: {
			double value = static_cast<DurationInput*>(rowIter->_widgets[0].get())->Value();
			_propertySet->SetDuration(property, value);
			} break;
		case Property::Integer: {
			std::string entryText = static_cast<Gtk::Entry*>(rowIter->_widgets[1].get())->get_text();
			_propertySet->SetInteger(property, std::atoi(entryText.c_str()));
			} break;
		}
		++rowIter;
	}
}
