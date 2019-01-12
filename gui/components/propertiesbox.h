#ifndef PROPERTIES_BOX_H
#define PROPERTIES_BOX_H

#include "../properties/propertyset.h"

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
	
	void SetPropertySet(PropertySet* propertySet);
	
private:
	struct PropertyRow {
		Property* _property;
		std::vector<std::unique_ptr<Gtk::Widget>> _widgets;
	};
	
	std::vector<PropertyRow> _rows;
	Gtk::Grid _propertiesRightGrid;
	Gtk::Label
		_thresholdLowerStartLabel,
		_thresholdLowerEndLabel,
		_thresholdUpperStartLabel,
		_thresholdUpperEndLabel;
	Gtk::Entry
		_thresholdLowerStart,
		_thresholdLowerEnd,
		_thresholdUpperStart,
		_thresholdUpperEnd;
	Gtk::ButtonBox _propertiesButtonBox;
	Gtk::Button _applyPropertiesButton;
	
	void onApplyPropertiesClicked();
};

#endif
