#ifndef PRESETVALUE_H
#define PRESETVALUE_H

#include "controlvalue.h"

#include <sigc++/signal.h>

#include <string>

/**
	@author Andre Offringa
*/
class PresetValue {
	public:
		PresetValue(class Controllable &controllable, size_t inputIndex)
			: _value(0), _controllable(&controllable), _inputIndex(inputIndex)
		{ }
		
		PresetValue(const PresetValue &source) = default;
		
		/**
		 * Copy constructor that copies the source but associates it with the given controllable.
		 */
		PresetValue(const PresetValue &source, class Controllable &controllable) :
			_value(source._value), _controllable(&controllable), _inputIndex(source._inputIndex)
		{ }
		
		~PresetValue() { _signalDelete(); }

		void SetValue(const ControlValue &value) { _value = value; }
		const ControlValue& Value() const { return _value; }
		ControlValue& Value() { return _value; }

		class Controllable& Controllable() const { return *_controllable; }
		
		size_t InputIndex() const { return _inputIndex; }
		
		bool IsIgnorable() const { return _value.UInt() == 0; }
		
		sigc::signal<void()>& SignalDelete() { return _signalDelete; }
		
		std::string Title() const;
		
	private:
		ControlValue _value;
		class Controllable *_controllable;
		size_t _inputIndex;
		
		sigc::signal<void()> _signalDelete;
};

#endif
