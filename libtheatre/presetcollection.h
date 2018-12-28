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
		PresetCollection() { }
		~PresetCollection() { Clear(); }

		void Clear()
		{
			_presetValues.clear();
		}
		inline void SetFromCurrentSituation(const Management &management);

		virtual void Mix(const ControlValue &value, unsigned *channelValues, unsigned universe, const Timing& timing) final override
		{
			unsigned leftHand = value.UInt();
			for(const std::unique_ptr<PresetValue>& pv : _presetValues)
			{
				unsigned rightHand = pv->Value().UInt();
				ControlValue value(ControlValue::Mix(leftHand, rightHand, ControlValue::Multiply));

				pv->Controllable().Mix(value, channelValues, universe, timing);
			}
		}
		const std::vector<std::unique_ptr<PresetValue>> &PresetValues() const { return _presetValues; }
		PresetValue& AddPresetValue(unsigned id, class Controllable &controllable)
		{
			_presetValues.emplace_back(new PresetValue(id, controllable));
			return *_presetValues.back();
		}
		bool IsUsing(const Controllable &controllable) const
		{
			for(const std::unique_ptr<PresetValue>& pv : _presetValues)
				if((&pv->Controllable()) == &controllable) return true;
			return false;
		}
	private:
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
