#ifndef FIXTURE_CONTROL_H
#define FIXTURE_CONTROL_H

#include "controllable.h"
#include "fixture.h"

class FixtureControl : public Controllable
{
public:
	FixtureControl(class Fixture& fixture) :
		Controllable(fixture.Name()),
		_fixture(&fixture),
		_values(fixture.Functions().size())
	{ }
	
	class Fixture &Fixture() const
	{
		return *_fixture;
	}
	
	size_t NInputs() const final override
	{ return _fixture->Functions().size(); }
	
	ControlValue& InputValue(size_t index) final override
	{ return _values[index]; }
	
	size_t NOutputs() const final override
	{ return 0; }
	
	std::pair<Controllable*, size_t> Output(size_t) final override
	{ return std::pair<Controllable*, size_t>(nullptr, 0); }
	
	void Mix(unsigned int *, unsigned int, const class Timing &) final override { }
		
private:
	class Fixture *_fixture;
	std::vector<ControlValue> _values;
};

#endif
