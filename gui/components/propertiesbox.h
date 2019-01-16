#ifndef PROPERTIES_BOX_H
#define PROPERTIES_BOX_H

#include "../../libtheatre/properties/propertyset.h"

#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/widget.h>

#include <memory>
#include <vector>

class PropertiesBox : public Gtk::VBox
{
public:
	PropertiesBox();
	
	void SetPropertySet(PropertySet* propertySet)
	{ _propertySet = propertySet; fillProperties(); }
	
	void Clear();
	
private:
	struct PropertyRow {
		Property* _property;
		std::vector<std::unique_ptr<Gtk::Widget>> _widgets;
	};
	
	PropertySet* _propertySet;
	std::vector<PropertyRow> _rows;
	Gtk::Label _typeLabel;
	Gtk::Grid _grid;
	Gtk::ButtonBox _propertiesButtonBox;
	Gtk::Button _applyButton;
	
	void onApplyClicked();
	void fillProperties();
};

#endif
