#ifndef FIXTURE_SELECTION_H
#define FIXTURE_SELECTION_H

#include <sigc++/signal.h>

#include <vector>

class FixtureSelection
{
public:
	sigc::signal<void>& SignalChange() { return _signalChange; }
	
	const std::vector<class Fixture*>& Selection() const { return _selection; }
	
	void SetSelection(const std::vector<class Fixture*>& selection)
	{
		_selection = selection;
		_signalChange.emit();
	}
	void SetSelection(std::vector<class Fixture*>&& selection)
	{
		_selection = std::move(selection);
		_signalChange.emit();
	}
	
private:
	sigc::signal<void> _signalChange;
	std::vector<class Fixture*> _selection;
};

#endif
