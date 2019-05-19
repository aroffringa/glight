#ifndef PRESETCOLLECTION_H
#define PRESETCOLLECTION_H

#include <vector>
#include <memory>

#include "controllable.h"
#include "dmxchannel.h"
#include "presetvalue.h"

/**
	@author Andre Offringa
*/
class PresetCollection : public Controllable {
	public:
		PresetCollection() : _inputValue(0) { }
		PresetCollection(const std::string& name) : Controllable(name), _inputValue(0) { }
		~PresetCollection() { Clear(); }

		void Clear()
		{
			_presetValues.clear();
		}
		inline void SetFromCurrentSituation(const Management &management);

		size_t NInputs() const final override
		{ return 1; }
		
		ControlValue& InputValue(size_t index) final override
		{ return _inputValue; }
		
		size_t NOutputs() const final override
		{ return _presetValues.size(); }
		
		std::pair<Controllable*, size_t> Output(size_t index) final override
		{ return std::make_pair(&_presetValues[index]->Controllable(), _presetValues[index]->InputIndex()); }
		
		void Mix(unsigned *channelValues, unsigned universe, const Timing& timing) final override
		{
			unsigned leftHand = _inputValue.UInt();
			for(const std::unique_ptr<PresetValue>& pv : _presetValues)
			{
				unsigned rightHand = pv->Value().UInt();
				ControlValue value(ControlValue::Mix(leftHand, rightHand, ControlValue::Multiply));

				pv->Controllable().MixInput(pv->InputIndex(), value);
			}
		}
		const std::vector<std::unique_ptr<PresetValue>> &PresetValues() const { return _presetValues; }
		PresetValue& AddPresetValue(class PresetValue& source)
		{
			_presetValues.emplace_back(new PresetValue(source));
			return *_presetValues.back();
		}
		PresetValue& AddPresetValue(unsigned id, class Controllable &controllable, size_t input)
		{
			_presetValues.emplace_back(new PresetValue(id, controllable, input));
			return *_presetValues.back();
		}
		PresetValue& AddPresetValue(class PresetValue& source, class Controllable &controllable)
		{
			_presetValues.emplace_back(new PresetValue(source, controllable));
			return *_presetValues.back();
		}
		bool IsUsing(const Controllable &controllable) const
		{
			for(const std::unique_ptr<PresetValue>& pv : _presetValues)
				if((&pv->Controllable()) == &controllable) return true;
			return false;
		}
	private:
		ControlValue _inputValue;
		std::vector<std::unique_ptr<PresetValue>> _presetValues;
};

#include "management.h"

void PresetCollection::SetFromCurrentSituation(const Management &management)
{
	Clear();
	const std::vector<std::unique_ptr<PresetValue>>&
		values = management.PresetValues();
	for(const std::unique_ptr<PresetValue>& pv : values)
	{
		if(!pv->IsIgnorable() && (&pv->Controllable()) != this)
			_presetValues.emplace_back(new PresetValue(*pv));
	}
}

#endif
