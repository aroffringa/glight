#ifndef DURATION_INPUT_H
#define DURATION_INPUT_H

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../recursionlock.h"

class DurationInput : public Gtk::HBox
{
public:
	DurationInput(double value);
	
	DurationInput(const std::string& label, double value);
	
	sigc::signal<void(double)>& SignalValueChanged() { return _signalValueChanged; }
	
	double Value() const { return atof(_entry.get_text().c_str())*1e3; }
	
	void SetValue(double newValue);
	
private:
	void initialize(double value);
	
	static double valueToScale(double value);
	void onScaleChanged();
	void onEntryChanged();
	void setEntry(double value);
	
	static constexpr size_t NVALUES = 21;
	static const double values[NVALUES];

	Gtk::Label _label;
	Gtk::HScale _scale;
	Gtk::Entry _entry;
	RecursionLock _recursionLock;
	sigc::signal<void(double)> _signalValueChanged;
};

#endif
