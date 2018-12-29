#ifndef FIXTUREFUNCTIONCONTROL_H
#define FIXTUREFUNCTIONCONTROL_H

#include "controllable.h"
#include "fixturefunction.h"

/**
	@author André Offringa
*/
class FixtureFunctionControl : public Controllable {
	public:
		FixtureFunctionControl(class FixtureFunction &function)
			: Controllable(function.Name()), _function(&function)
		{ }

		virtual void Mix(const ControlValue &value, unsigned *channelValues, unsigned universe, const class Timing&)
		{
			_function->Mix(value.UInt(), ControlValue::Default, channelValues, universe);
		}

		class FixtureFunction &Function() const
		{
			return *_function;
		}
	private:
		class FixtureFunction *_function;
};

#endif
