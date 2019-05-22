#include "fixture.h"
#include "theatre.h"

Fixture::Fixture(Theatre &theatre, FixtureType &type, const std::string &name) : FolderObject(name), _theatre(theatre), _type(type)
{
	size_t ch = theatre.FirstFreeChannel();
	
	switch(type.FixtureClass())
	{
		case FixtureType::Light1Ch:
		{
			FixtureFunction *f =
				new FixtureFunction(_theatre, FixtureFunction::Brightness, name);
			_functions.emplace_back(f);
		}
		break;
		case FixtureType::RGBLight3Ch:
		{
			FixtureFunction *r, *g, *b;
			r = new FixtureFunction(_theatre, FixtureFunction::RedIntensity, name + "_R");
			g = new FixtureFunction(_theatre, FixtureFunction::GreenIntensity, name + "_G");
			b = new FixtureFunction(_theatre, FixtureFunction::BlueIntensity, name + "_B");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
		}
		break;
		case FixtureType::RGBLight4Ch:
		{
			FixtureFunction *r, *g, *b, *m;
			r = new FixtureFunction(_theatre, FixtureFunction::RedIntensity, name + "_R");
			g = new FixtureFunction(_theatre, FixtureFunction::GreenIntensity, name + "_G");
			b = new FixtureFunction(_theatre, FixtureFunction::BlueIntensity, name + "_B");
			m = new FixtureFunction(_theatre, FixtureFunction::Brightness, name + "_M");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
			_functions.emplace_back(m);
		}
		break;
		case FixtureType::RGBALight4Ch:
		{
			FixtureFunction *r, *g, *b, *a;
			r = new FixtureFunction(_theatre, FixtureFunction::RedIntensity, name + "_R");
			g = new FixtureFunction(_theatre, FixtureFunction::GreenIntensity, name + "_G");
			b = new FixtureFunction(_theatre, FixtureFunction::BlueIntensity, name + "_B");
			a = new FixtureFunction(_theatre, FixtureFunction::AmberIntensity, name + "_A");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
			_functions.emplace_back(a);
		}
		break;
		case FixtureType::RGBALight5Ch:
		{
			FixtureFunction *r, *g, *b, *a, *m;
			r = new FixtureFunction(_theatre, FixtureFunction::RedIntensity, name + "_R");
			g = new FixtureFunction(_theatre, FixtureFunction::GreenIntensity, name + "_G");
			b = new FixtureFunction(_theatre, FixtureFunction::BlueIntensity, name + "_B");
			a = new FixtureFunction(_theatre, FixtureFunction::AmberIntensity, name + "_A");
			m = new FixtureFunction(_theatre, FixtureFunction::Brightness, name + "_M");
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
			r = new FixtureFunction(_theatre, FixtureFunction::RedIntensity, name + "_R");
			g = new FixtureFunction(_theatre, FixtureFunction::GreenIntensity, name + "_G");
			b = new FixtureFunction(_theatre, FixtureFunction::BlueIntensity, name + "_B");
			w = new FixtureFunction(_theatre, FixtureFunction::WhiteIntensity, name + "_W");
			_functions.emplace_back(r);
			_functions.emplace_back(g);
			_functions.emplace_back(b);
			_functions.emplace_back(w);
		}
		break;
		case FixtureType::UVLight3Ch:
		{
			FixtureFunction *m, *s, *p;
			m = new FixtureFunction(_theatre, FixtureFunction::Brightness, name + "_M");
			s = new FixtureFunction(_theatre, FixtureFunction::Strobe, name + "_S");
			p = new FixtureFunction(_theatre, FixtureFunction::Pulse, name + "_P");
			_functions.emplace_back(m);
			_functions.emplace_back(s);
			_functions.emplace_back(p);
		}
		break;
		case FixtureType::H2ODMXPro:
		{
			FixtureFunction *m, *r, *c;
			m = new FixtureFunction(_theatre, FixtureFunction::Brightness, name + "_M");
			r = new FixtureFunction(_theatre, FixtureFunction::Rotation, name + "_R");
			c = new FixtureFunction(_theatre, FixtureFunction::ColorMacro, name + "_C");
			_functions.emplace_back(m);
			_functions.emplace_back(r);
			_functions.emplace_back(c);
		}
		break;
		case FixtureType::RGB_ADJ_6CH:
		{
			FixtureFunction *r, *g, *b, *c, *s, *p;
			r = new FixtureFunction(_theatre, FixtureFunction::RedIntensity, name + "_R");
			g = new FixtureFunction(_theatre, FixtureFunction::GreenIntensity, name + "_G");
			b = new FixtureFunction(_theatre, FixtureFunction::BlueIntensity, name + "_B");
			c = new FixtureFunction(_theatre, FixtureFunction::ColorMacro, name + "_C");
			s = new FixtureFunction(_theatre, FixtureFunction::Strobe, name + "_S");
			p = new FixtureFunction(_theatre, FixtureFunction::Pulse, name + "_P");
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
			r = new FixtureFunction(_theatre, FixtureFunction::RedIntensity, name + "_R");
			g = new FixtureFunction(_theatre, FixtureFunction::GreenIntensity, name + "_G");
			b = new FixtureFunction(_theatre, FixtureFunction::BlueIntensity, name + "_B");
			c = new FixtureFunction(_theatre, FixtureFunction::ColorMacro, name + "_C");
			s = new FixtureFunction(_theatre, FixtureFunction::Strobe, name + "_S");
			p = new FixtureFunction(_theatre, FixtureFunction::Pulse, name + "_P");
			m = new FixtureFunction(_theatre, FixtureFunction::Brightness, name + "_M");
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
	FolderObject(source), _theatre(theatre), _type(theatre.GetFixtureType(source._type.Name()))
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
