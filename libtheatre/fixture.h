#ifndef FIXTURE_H
#define FIXTURE_H

#include <set>

#include "color.h"
#include "fixturefunction.h"
#include "namedobject.h"

/**
	@author Andre Offringa
*/
class Fixture : public NamedObject {
	public:
		Fixture(class Theatre &theatre, class FixtureType &type, const std::string &name);
		Fixture(const Fixture& source, class Theatre& theatre);
		~Fixture();

		const std::vector<std::unique_ptr<FixtureFunction>> &Functions() const
		{ return _functions; }
		
		FixtureType &Type() const { return _type; }
		std::set<unsigned> GetChannels() const
		{
			std::set<unsigned> channels;
			for(const std::unique_ptr<FixtureFunction>& ff : _functions)
			{
				if(ff->IsSingleChannel())
					channels.insert(ff->FirstChannel().Channel());
			}
			return channels;
		}
		void IncChannel();

		void DecChannel();
		
		void SetChannel(unsigned dmxChannel);

		void ClearFunctions()
		{
			_functions.clear();
		}
		FixtureFunction& AddFunction(FixtureFunction::FunctionType type)
		{
			_functions.emplace_back(new FixtureFunction(_theatre, type));
			return *_functions.back();
		}
		inline Color GetColor(const class ValueSnapshot &snapshot) const;
	private:
		class Theatre &_theatre;
		FixtureType &_type;
		std::vector<std::unique_ptr<FixtureFunction>> _functions;
};

#include "fixturetype.h"

Color Fixture::GetColor(const class ValueSnapshot &snapshot) const
{
	return _type.GetColor(*this, snapshot);
}

#endif
