#include "theatre.h"

#include "fixture.h"

#include <sstream>

Theatre::Theatre(const Theatre& source) : _highestChannel(source._highestChannel)
{
	_fixtureTypes.reserve(source._fixtureTypes.size());
	for(const std::unique_ptr<FixtureType>& fixtureType : source._fixtureTypes)
		_fixtureTypes.emplace_back(new FixtureType(*fixtureType));
	
	_fixtures.reserve(source._fixtures.size());
	for(const std::unique_ptr<Fixture>& fixture : source._fixtures)
		_fixtures.emplace_back(new Fixture(*fixture, *this));
}

void Theatre::Clear()
{
	_fixtures.clear();
	_fixtureTypes.clear();
}

Fixture& Theatre::AddFixture(FixtureType &type)
{
	// Find free name
	std::string name;
	bool found = false;
	for(size_t i=0; i!=26; ++i)
	{
		name = std::string(1, (char) ('A' + i));
		if(!NamedObject::Contains(_fixtures, name))
		{
			found = true;
			break;
		}
	}
	size_t number = 1;
	while(!found)
	{
		std::stringstream s;
		s << type.Name() << number;
		name = s.str();
		if(!NamedObject::Contains(_fixtures, name))
			found = true;
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

void Theatre::RemoveFixture(Fixture& fixture)
{
	FixtureType* t = &fixture.Type();
	
	size_t fIndex = NamedObject::FindIndex(_fixtures, &fixture);
	_fixtures.erase(_fixtures.begin() + fIndex);
	
	if(!IsUsed(*t))
	{
		size_t ftIndex = NamedObject::FindIndex(_fixtureTypes, t);
		_fixtureTypes.erase(_fixtureTypes.begin() + ftIndex);
	}
}

bool Theatre::IsUsed(FixtureType& fixtureType) const
{
	for(const std::unique_ptr<Fixture>& f : _fixtures)
	{
		if(&f->Type() == &fixtureType)
		{
			return true;
		}
	}
	return false;
}

void Theatre::NotifyDmxChange()
{
	unsigned highest = 0;
	for(const std::unique_ptr<class Fixture>& fixture : _fixtures)
	{
		std::vector<unsigned> channels = fixture->GetChannels();
		for(unsigned channel : channels)
		{
			if(channel > highest)
				highest = channel;
		}
	}
	_highestChannel = highest;
}

