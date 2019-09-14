#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <memory>

#include <gtkmm/bin.h>

/**
	@author Andre Offringa
	Base class for GUI controls that allow switching presets, and that
	should obey fading or solo settings.
*/
class ControlWidget : public Gtk::Bin {
public:

	ControlWidget() :
		_fadingValue(0),
		_targetValue(0)
	{ }

	virtual void Toggle() = 0;
	virtual void FullOn() = 0;
	virtual void FullOff() = 0;
	virtual void Assign(class PresetValue* item, bool moveFader) = 0;
	void Unassign() { Assign(nullptr, false); }
	virtual class PresetValue* Preset() const = 0;
	
	virtual sigc::signal<void, double>& SignalValueChange() { return _signalValueChange; }
	virtual sigc::signal<void>& SignalAssigned() { return _signalAssigned; }
	
	virtual void Limit(double value) = 0;
	
	void SetFadeUpSpeed(double fadePerSecond) { _fadeUpSpeed = fadePerSecond; }
	void SetFadeDownSpeed(double fadePerSecond) { _fadeDownSpeed = fadePerSecond; }
	
	virtual void ChangeManagement(class Management& management, bool moveSliders) = 0;

	static double MAX_SCALE_VALUE();
	
	void writeValue(unsigned target);
	void UpdateValue(double timePassed);
	
protected:
	double _fadeUpSpeed, _fadeDownSpeed;
	
	void immediateAssign(unsigned value)
	{
		_fadingValue = value;
		_targetValue = value;
	}
	
private:
	unsigned _fadingValue, _targetValue;
	
	sigc::signal<void, double> _signalValueChange;
	sigc::signal<void> _signalAssigned;

};

#endif
