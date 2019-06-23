#include "fixture.h"
#include "theatre.h"

Fixture::Fixture(Theatre &theatre, FixtureType &type, const std::string &name) : NamedObject(name), _theatre(theatre), _type(type)
{
	size_t ch = theatre.FirstFreeChannel();
	
	switch(type.FixtureClass())
	{
		case FixtureType::Light1Ch:
		{
			FixtureFunction *f =
				new FixtureFunction(_theatre, FunctionType::Master, name);
			_functions.emplace_back(f);
		}
		break;
		case FixtureType::RGBLight3Ch:
		{
			FixtureFunction *r, *g, *b;
			r = new FixtureFunction(_theatre, FunctionType::Red, "R");
			g = new FixtureFunction(_theatre, FunctionType::Green, "G");
			b = new FixtureFunction(_theatre, FunctionType::Blue, "B");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
		}
		break;
		case FixtureType::RGBLight4Ch:
		{
			FixtureFunction *r, *g, *b, *m;
			r = new FixtureFunction(_theatre, FunctionType::Red, "R");
			g = new FixtureFunction(_theatre, FunctionType::Green, "G");
			b = new FixtureFunction(_theatre, FunctionType::Blue, "B");
			m = new FixtureFunction(_theatre, FunctionType::Master, "M");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
			_functions.emplace_back(m);
		}
		break;
		case FixtureType::RGBALight4Ch:
		{
			FixtureFunction *r, *g, *b, *a;
			r = new FixtureFunction(_theatre, FunctionType::Red, "R");
			g = new FixtureFunction(_theatre, FunctionType::Green, "G");
			b = new FixtureFunction(_theatre, FunctionType::Blue, "B");
			a = new FixtureFunction(_theatre, FunctionType::Amber, "A");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
			_functions.emplace_back(a);
		}
		break;
		case FixtureType::RGBALight5Ch:
		{
			FixtureFunction *r, *g, *b, *a, *m;
			r = new FixtureFunction(_theatre, FunctionType::Red, "R");
			g = new FixtureFunction(_theatre, FunctionType::Green, "G");
			b = new FixtureFunction(_theatre, FunctionType::Blue, "B");
			a = new FixtureFunction(_theatre, FunctionType::Amber, "A");
			m = new FixtureFunction(_theatre, FunctionType::Master, "M");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
			_functions.emplace_back(a);
			_functions.emplace_back(m);
		}
		break;
		case FixtureType::RGBWLight4Ch:
		{
			FixtureFunction *r, *g, *b, *w;
			r = new FixtureFunction(_theatre, FunctionType::Red, "R");
			g = new FixtureFunction(_theatre, FunctionType::Green, "G");
			b = new FixtureFunction(_theatre, FunctionType::Blue, "B");
			w = new FixtureFunction(_theatre, FunctionType::White, "W");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
			_functions.emplace_back(w);
		}
		break;
		case FixtureType::UVLight3Ch:
		{
			FixtureFunction *m, *s, *p;
			m = new FixtureFunction(_theatre, FunctionType::Master, "M");
			s = new FixtureFunction(_theatre, FunctionType::Strobe, "S");
			p = new FixtureFunction(_theatre, FunctionType::Pulse, "P");
			_functions.emplace_back(m);
			_functions.emplace_back(s);
			_functions.emplace_back(p);
		}
		break;
		case FixtureType::H2ODMXPro:
		{
			FixtureFunction *m, *r, *c;
			m = new FixtureFunction(_theatre, FunctionType::Master, "M");
			r = new FixtureFunction(_theatre, FunctionType::Rotation, "R");
			c = new FixtureFunction(_theatre, FunctionType::ColorMacro, "C");
			_functions.emplace_back(m);
			_functions.emplace_back(r);
			_functions.emplace_back(c);
		}
		break;
		case FixtureType::RGB_ADJ_6CH:
		{
			FixtureFunction *r, *g, *b, *c, *s, *p;
			r = new FixtureFunction(_theatre, FunctionType::Red, "R");
			g = new FixtureFunction(_theatre, FunctionType::Green, "G");
			b = new FixtureFunction(_theatre, FunctionType::Blue, "B");
			c = new FixtureFunction(_theatre, FunctionType::ColorMacro, "C");
			s = new FixtureFunction(_theatre, FunctionType::Strobe, "S");
			p = new FixtureFunction(_theatre, FunctionType::Pulse, "P");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
			_functions.emplace_back(c);
			_functions.emplace_back(s);
			_functions.emplace_back(p);
		}
		break;
		case FixtureType::RGB_ADJ_7CH:
		{
			FixtureFunction *r, *g, *b, *c, *s, *p, *m;
			r = new FixtureFunction(_theatre, FunctionType::Red, "R");
			g = new FixtureFunction(_theatre, FunctionType::Green, "G");
			b = new FixtureFunction(_theatre, FunctionType::Blue, "B");
			c = new FixtureFunction(_theatre, FunctionType::ColorMacro, "C");
			s = new FixtureFunction(_theatre, FunctionType::Strobe, "S");
			p = new FixtureFunction(_theatre, FunctionType::Pulse, "P");
			m = new FixtureFunction(_theatre, FunctionType::Master, "M");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
			_functions.emplace_back(c);
			_functions.emplace_back(s);
			_functions.emplace_back(p);
			_functions.emplace_back(m);
			break;
		}
	}
	
	for(size_t ci=0; ci!=_functions.size(); ++ci)
	{
		_functions[ci]->SetChannel(DmxChannel(ch + ci, 0));
	}
}

Fixture::Fixture(const Fixture& source, class Theatre& theatre) :
	NamedObject(source),
	_theatre(theatre),
	_type(theatre.GetFixtureType(source._type.Name())),
	_position(source.Position())
{
	for(const std::unique_ptr<FixtureFunction>& ff : source._functions)
		_functions.emplace_back(new FixtureFunction(*ff, theatre));
}

void Fixture::IncChannel()
{
	for(std::unique_ptr<FixtureFunction>& ff : _functions)
		ff->IncChannel();

	_theatre.NotifyDmxChange();
}

void Fixture::DecChannel()
{
	for(std::unique_ptr<FixtureFunction>& ff : _functions)
		ff->DecChannel();
	
	_theatre.NotifyDmxChange();
}

void Fixture::SetChannel(unsigned dmxChannel)
{
	for(std::unique_ptr<FixtureFunction>& ff : _functions)
	{
		dmxChannel = dmxChannel % 512;
		DmxChannel c = ff->FirstChannel();
		c.SetChannel(dmxChannel);
		ff->SetChannel(c);
		if(ff->IsSingleChannel())
			++dmxChannel;
		else
			dmxChannel += 2;
	}
	
	_theatre.NotifyDmxChange();
}
