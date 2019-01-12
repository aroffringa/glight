#include "propertiesbox.h"

#include <gtkmm/stock.h>

PropertiesBox::PropertiesBox() :
	_thresholdLowerStartLabel("Lower start:"),
	_thresholdLowerEndLabel("Lower end:"),
	_thresholdUpperStartLabel("Upper start:"),
	_thresholdUpperEndLabel("Upper end:"),
	_applyPropertiesButton(Gtk::Stock::APPLY)
{
	_propertiesRightGrid.attach(_thresholdLowerStartLabel, 0, 0);
	_propertiesRightGrid.attach(_thresholdLowerStart, 1, 0);
	_propertiesRightGrid.attach(_thresholdLowerEndLabel, 0, 1);
	_propertiesRightGrid.attach(_thresholdLowerEnd, 1, 1);
	_propertiesRightGrid.attach(_thresholdUpperStartLabel, 0, 2);
	_propertiesRightGrid.attach(_thresholdUpperStart, 1, 2);
	_propertiesRightGrid.attach(_thresholdUpperEndLabel, 0, 3);
	_propertiesRightGrid.attach(_thresholdUpperEnd, 1, 3);
	pack_start(_propertiesRightGrid);
	
	_applyPropertiesButton.signal_clicked().connect(sigc::mem_fun(*this, &PropertiesBox::onApplyPropertiesClicked));
	
	_propertiesButtonBox.pack_start(_applyPropertiesButton);
	pack_end(_propertiesButtonBox);
}

