#ifndef VALUESNAPSHOT_H
#define VALUESNAPSHOT_H

#include "dmxchannel.h"
#include "valueuniversesnapshot.h"

class ValueSnapshot
{
	public:
		ValueSnapshot()
		{
		}
		ValueSnapshot(const ValueSnapshot &source)
		{
			for(std::vector<ValueUniverseSnapshot*>::const_iterator i=source._universeValues.begin();i!=source._universeValues.end();++i)
				_universeValues.push_back(new ValueUniverseSnapshot(**i));
		}
		ValueSnapshot &operator=(const ValueSnapshot &rhs)
		{
			Clear();
			for(std::vector<ValueUniverseSnapshot*>::const_iterator i=rhs._universeValues.begin();i!=rhs._universeValues.end();++i)
				_universeValues.push_back(new ValueUniverseSnapshot(**i));
			return *this;
		}
		~ValueSnapshot()
		{
			Clear();
		}
		void Clear()
		{
			for(std::vector<ValueUniverseSnapshot*>::iterator i=_universeValues.begin();i!=_universeValues.end();++i)
				delete *i;
			_universeValues.clear();
		}
		void SetUniverseCount(size_t count)
		{
			while(count < _universeValues.size())
			{
				_universeValues.erase(--_universeValues.end());
			}
			while(count > _universeValues.size())
			{
				_universeValues.push_back(new ValueUniverseSnapshot());
			}
		}
		unsigned char GetValue(const DmxChannel &channel) const
		{
			return _universeValues[channel.Universe()]->GetValue(channel.Channel());
		}
		ValueUniverseSnapshot &GetUniverseSnapshot(size_t index) const
		{
			return *_universeValues[index];
		}
	private:
		std::vector<ValueUniverseSnapshot*> _universeValues;
};

#endif // VALUESNAPSHOT_H
