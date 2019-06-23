#ifndef FIXTURE_H
#define FIXTURE_H

#include <set>

#include "color.h"
#include "fixturefunction.h"
#include "namedobject.h"
#include "position.h"

/**
	@author Andre Offringa
*/
class Fixture : public NamedObject {
public:
	Fixture(class Theatre &theatre, class FixtureType &type, const std::string &name);
	Fixture(const Fixture& source, class Theatre& theatre);

	const std::vector<std::unique_ptr<FixtureFunction>> &Functions() const
	{ return _functions; }
	
	FixtureType &Type() const { return _type; }
	
	std::vector<unsigned> GetChannels() const
	{
		std::vector<unsigned> channels;
		for(const std::unique_ptr<FixtureFunction>& ff : _functions)
		{
			if(ff->IsSingleChannel())
				channels.emplace_back(ff->FirstChannel().Channel());
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
	FixtureFunction& AddFunction(FunctionType type)
	{
		_functions.emplace_back(new FixtureFunction(_theatre, type));
		return *_functions.back();
	}
	inline Color GetColor(const class ValueSnapshot &snapshot) const;
	
	class Position& Position() { return _position; }
	const class Position& Position() const { return _position; }
	
private:
	class Theatre& _theatre;
	FixtureType& _type;
	class Position _position;
	std::vector<std::unique_ptr<FixtureFunction>> _functions;
};

#include "fixturetype.h"

Color Fixture::GetColor(const class ValueSnapshot &snapshot) const
{
	return _type.GetColor(*this, snapshot);
}

#endif
