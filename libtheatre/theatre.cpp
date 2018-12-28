#include "theatre.h"

#include "fixture.h"

#include <sstream>

void Theatre::Clear()
{
	_fixtures.clear();
	_fixtureTypes.clear();
}

Fixture& Theatre::AddFixture(FixtureType &type)
{
	std::string name;
	if(_fixtures.size() < 26)
		name = std::string(1, (char) ('A' + _fixtures.size()));
	else
	{
		std::stringstream s;
		s << type.Name() << (_fixtures.size() + 1);
		name = s.str();
	}
	_fixtures.emplace_back(new Fixture(*this, type, name));
	Fixture& f = *_fixtures.back();
	NotifyDmxChange();
	return f;
}

FixtureType& Theatre::AddFixtureType(enum FixtureType::FixtureClass fixtureClass)
{
	_fixtureTypes.emplace_back(new FixtureType(fixtureClass));
	return *_fixtureTypes.back();
}

Fixture& Theatre::GetFixture(const std::string &name) const
{
	return NamedObject::FindNamedObject(_fixtures, name);
}

FixtureType& Theatre::GetFixtureType(const std::string &name) const
{
	return NamedObject::FindNamedObject(_fixtureTypes, name);
}

FixtureFunction& Theatre::GetFixtureFunction(const std::string &name) const
{
	for(const std::unique_ptr<Fixture>& f : _fixtures)
	{
		const std::vector<std::unique_ptr<FixtureFunction>>& functions = f->Functions();
		for(const std::unique_ptr<FixtureFunction>& function : functions)
		{
			if(function->Name() == name)
				return *function;
		}
	}
	throw std::runtime_error(std::string("Can not find fixture function with name ") + name);
}

void Theatre::NotifyDmxChange()
{
	unsigned highest = 0;
	for(const std::unique_ptr<class Fixture>& fixture : _fixtures)
	{
		std::set<unsigned> channels = fixture->GetChannels();
		unsigned curHighest = *channels.rbegin();
		if(curHighest > highest)
			highest = curHighest;
	}
	_highestChannel = highest;
}

