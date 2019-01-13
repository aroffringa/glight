#include "propertiesbox.h"

#include <gtkmm/stock.h>

PropertiesBox::PropertiesBox() :
	_typeLabel("No object selected"),
	_applyButton(Gtk::Stock::APPLY)
{
	pack_start(_typeLabel);
	pack_start(_grid);
	
	_applyButton.signal_clicked().connect(sigc::mem_fun(*this, &PropertiesBox::onApplyClicked));
	_propertiesButtonBox.pack_start(_applyButton);
	
	pack_end(_propertiesButtonBox);
	
	show_all_children();
}

void PropertiesBox::fillProperties()
{
	_rows.clear();
	_typeLabel.set_text(_propertySet->GetTypeDescription());
	for(Property& property : *_propertySet)
	{
		size_t rowIndex = _rows.size();
		_rows.emplace_back();
		PropertyRow& row = _rows.back();
		
		switch(property.GetType())
		{
			case Property::ControlValue: {
				std::string entryText =
					std::to_string(100.0*_propertySet->GetControlValue(property)/ControlValue::MaxUInt());
					
				row._widgets.emplace_back(new Gtk::Label(property.Title()));
				_grid.attach(*row._widgets.back(), 0, rowIndex, 1, 1);
				
				Gtk::Entry* entry = new Gtk::Entry();
				row._widgets.emplace_back(entry);
				entry->set_text(entryText);
				_grid.attach(*entry, 1, rowIndex, 1, 1);
			}
		}
	}
	_grid.show_all_children();
}

void PropertiesBox::onApplyClicked()
{
	auto rowIter = _rows.begin();
	for(Property& property : *_propertySet)
	{
		switch(property.GetType())
		{
		case Property::ControlValue: {
			std::string entryText = static_cast<Gtk::Entry*>(rowIter->_widgets[1].get())->get_text();
			_propertySet->SetControlValue(property, unsigned(std::atof(entryText.c_str())*ControlValue::MaxUInt()/100.0));
			} break;
		}
		++rowIter;
	}
}
