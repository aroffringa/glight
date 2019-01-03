#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <vector>

#include "namedobject.h"

/**
	@author Andre Offringa
*/
class Sequence : public NamedObject {
	public:
		Sequence()
		{
		}

		~Sequence()
		{
		}
		
		std::unique_ptr<Sequence> CopyWithoutPresets()
		{
			return std::unique_ptr<Sequence>(new Sequence(static_cast<NamedObject&>(*this)));
		}

		size_t Size() const { return _presets.size(); }

		void AddPreset(class PresetCollection *preset)
		{
			_presets.push_back(preset);
		}

		const std::vector<class PresetCollection *> &Presets() const
		{
			return _presets;
		}

		bool IsUsing(class PresetCollection &presetCollection) const
		{
			for(std::vector<class PresetCollection *>::const_iterator i=_presets.begin();
				i!=_presets.end();++i)
				if(*i == &presetCollection) return true;
			return false;
		}
	private:
		Sequence(const NamedObject& namedObj) : NamedObject(namedObj)
		{ }
		
		std::vector<class PresetCollection *> _presets;
};

#endif
