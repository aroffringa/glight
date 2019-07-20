#include "durationinput.h"

#include <sstream>

const double DurationInput::values[NVALUES] =
{
	0.0, 40.0, 80.0, 160.0, 300.0,
	500.0, 750.0, 1000.0, 1500.0, 2000.0,
	2500.0, 3000.0, 4000.0, 5000.0, 7500.0,
	10000.0, 15000.0, 30000.0, 60000.0, 120000.0,
	300000.0
};

DurationInput::DurationInput(const std::string& label, double value) :
	_label(label),
	_scale(0, NVALUES, 1)
{
	_label.set_halign(Gtk::ALIGN_END);
	pack_start(_label, false, false);
	
	_scale.set_value(valueToScale(value));
	_scale.set_round_digits(0);
	_scale.set_draw_value(false);
	_scale.signal_value_changed().connect([&]() { onScaleChanged(); });
	pack_start(_scale, true, true);
	
	setEntry(value);
	_entry.set_max_length(6);
	_entry.set_width_chars(6);
	_entry.signal_changed().connect([&]() { onEntryChanged(); });
	pack_end(_entry, false, false);
	
	show_all_children();
}

double DurationInput::valueToScale(double value)
{
	for(size_t i=0; i!=NVALUES-1; ++i)
	{
		if(values[i+1] > value)
		{
			if(value - values[i] < values[i+1] - value)
				return i;
			else
				return i+1;
		}
	}
	return NVALUES-1;
}

void DurationInput::onScaleChanged()
{
	if(_recursionLock.IsFirst())
	{
		RecursionLock::Token token(_recursionLock);
		size_t index = _scale.get_value();
		double newValue = values[index];
		setEntry(newValue);
		_signalValueChanged.emit(newValue);
	}
}

void DurationInput::onEntryChanged()
{
	if(_recursionLock.IsFirst())
	{
		RecursionLock::Token token(_recursionLock);
		double newValue = atof(_entry.get_text().c_str())*1e3;
		_scale.set_value(valueToScale(newValue));
		_signalValueChanged.emit(newValue);
	}
}

void DurationInput::SetValue(double newValue)
{
	RecursionLock::Token token(_recursionLock);
	_scale.set_value(valueToScale(newValue));
	setEntry(newValue);
	_signalValueChanged.emit(newValue);
}

void DurationInput::setEntry(double newValue)
{
	std::ostringstream str;
	str << (round(newValue*0.1)*1e-2);
	_entry.set_text(str.str());
}
