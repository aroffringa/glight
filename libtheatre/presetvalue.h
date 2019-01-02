#ifndef PRESETVALUE_H
#define PRESETVALUE_H

#include "controlvalue.h"

/**
	@author Andre Offringa
*/
class PresetValue {
	public:
		PresetValue(unsigned id, class Controllable &controllable)
			: _id(id), _value(0), _controllable(&controllable)
		{ }
		
		PresetValue(const PresetValue &source) = default;
		
		/**
		 * Copy constructor that copies the source but associates it with the given controllable.
		 */
		PresetValue(const PresetValue &source, class Controllable &controllable) :
			_id(source._id), _value(source._value), _controllable(&controllable)
		{ }

		void SetValue(const ControlValue &value) { _value = value; }
		const ControlValue &Value() const { return _value; }
		ControlValue &Value() { return _value; }

		unsigned Id() const { return _id; }

		class Controllable &Controllable() const { return *_controllable; }
		bool IsIgnorable() const { return _value.UInt() == 0; }
		
	private:
		unsigned _id;
		ControlValue _value;
		class Controllable *_controllable;
};

#endif
