#ifndef FIXTURETYPE_H
#define FIXTURETYPE_H

#include <string>
#include <vector>

#include "color.h"
#include "namedobject.h"
#include "valuesnapshot.h"

/**
	@author Andre Offringa
*/
class FixtureType : public NamedObject {
	public:
		enum FixtureClass {
			Light1Ch,
			RGBLight3Ch,
			RGBLight4Ch,
			RGBALight4Ch,
			RGBALight5Ch,
			RGBWLight4Ch,
			UVLight3Ch,
			H2ODMXPro,
			RGB_ADJ_6CH,
			RGB_ADJ_7CH
		};

		FixtureType(FixtureClass fixtureClass) : NamedObject(ClassName(fixtureClass)), _class(fixtureClass)
		{ }
		
		FixtureType(const FixtureType& fixtureType) :
			NamedObject(fixtureType),
			_class(fixtureType._class)
			{ }

		static const std::string ClassName(FixtureClass fixtureClass)
		{
			switch(fixtureClass)
			{
				case Light1Ch:
					return "Light (1ch)";
				case RGBLight3Ch:
					return "RGB light (3ch)";
				case RGBLight4Ch:
					return "RGB light (4ch)";
				case RGBALight4Ch:
					return "RGBA light (4ch)";
				case RGBALight5Ch:
					return "RGBA light (5ch)";
				case RGBWLight4Ch:
					return "RGBW light (4ch)";
				case UVLight3Ch:
					return "UV light (3ch)";
				case H2ODMXPro:
					return "H2O DMX Pro";
				case RGB_ADJ_6CH:
					return "RGB ADJ (6ch)";
				case RGB_ADJ_7CH:
					return "RGB ADJ (7ch)";
			}
			return "Unknown fixture class";
		}
		
		static std::vector<enum FixtureClass> GetClassList()
		{
			std::vector<enum FixtureClass> list {
				Light1Ch,
				RGBLight3Ch,
				RGBLight4Ch,
				RGBALight4Ch,
				RGBALight5Ch,
				RGBWLight4Ch,
				UVLight3Ch,
				H2ODMXPro,
				RGB_ADJ_6CH,
				RGB_ADJ_7CH
			};
			return list;
		}
		
		Color GetColor(const class Fixture &fixture, const ValueSnapshot &snapshot) const;

		enum FixtureClass FixtureClass() const { return _class; }
		
	private:
		static Color rgbAdj6chColor(const class Fixture &fixture, const ValueSnapshot &snapshot);
		
		enum FixtureClass _class;
};

#include "fixture.h"

inline Color FixtureType::rgbAdj6chColor(const class Fixture &fixture, const ValueSnapshot &snapshot)
{
	unsigned macro = fixture.Functions()[3]->GetValue(snapshot);
	if(macro < 8)
	{
		return Color(
			fixture.Functions()[0]->GetValue(snapshot),
			fixture.Functions()[1]->GetValue(snapshot),
			fixture.Functions()[2]->GetValue(snapshot)
		);
	}
	else {
		return Color::ADJMacroColor(macro);
	}
}

inline Color FixtureType::GetColor(const Fixture &fixture, const ValueSnapshot &snapshot) const
{
	switch(_class)
	{
		case Light1Ch:
		//case MovingLight:
		//case SmokeMachine:
		//case FollowSpot:
			return Color::Gray(fixture.Functions()[0]->GetValue(snapshot));
		case RGBLight3Ch:
			return Color(
				fixture.Functions()[0]->GetValue(snapshot),
				fixture.Functions()[1]->GetValue(snapshot),
				fixture.Functions()[2]->GetValue(snapshot)
			);
		case RGBLight4Ch: {
			unsigned char master = fixture.Functions()[3]->GetValue(snapshot);
			return Color(
				(fixture.Functions()[0]->GetValue(snapshot)*master)/255,
				(fixture.Functions()[1]->GetValue(snapshot)*master)/255,
				(fixture.Functions()[2]->GetValue(snapshot)*master)/255
			);
		}
		case RGBALight4Ch:
		{
			unsigned char a = fixture.Functions()[3]->GetValue(snapshot);
			return Color(
				(fixture.Functions()[0]->GetValue(snapshot) + a*2/3)*3/5,
				(fixture.Functions()[1]->GetValue(snapshot) + a/3)*3/5,
				(fixture.Functions()[2]->GetValue(snapshot)*3/5)
			);
			break;
		}
		case RGBALight5Ch:
		{
			unsigned char a = fixture.Functions()[3]->GetValue(snapshot);
			unsigned char master = fixture.Functions()[4]->GetValue(snapshot);
			return Color(
				((fixture.Functions()[0]->GetValue(snapshot) + a*2/3)*3/5)*master/255,
				((fixture.Functions()[1]->GetValue(snapshot) + a/3)*3/5)*master/255,
				((fixture.Functions()[2]->GetValue(snapshot)*3/5))*master/255
			);
			break;
		}
		case RGBWLight4Ch:
		{
			unsigned char w = fixture.Functions()[3]->GetValue(snapshot);
			return Color(
				(fixture.Functions()[0]->GetValue(snapshot) + w/2)*2/3,
				(fixture.Functions()[1]->GetValue(snapshot) + w/2)*2/3,
				(fixture.Functions()[2]->GetValue(snapshot) + w/2)*2/3
			);
			break;
		}
		case UVLight3Ch:
		{
			unsigned char m = fixture.Functions()[0]->GetValue(snapshot);
			return Color(m/3, 0, m);
		}
		case H2ODMXPro:
		{
			return Color::H20Color(fixture.Functions()[2]->GetValue(snapshot)) *
				fixture.Functions()[0]->GetValue(snapshot);
		}
		case RGB_ADJ_6CH:
		{
			return rgbAdj6chColor(fixture, snapshot);
		}
		case RGB_ADJ_7CH:
		{
			Color c = rgbAdj6chColor(fixture, snapshot);
			unsigned char master = fixture.Functions()[6]->GetValue(snapshot);
			return c * master;
		}
	}
	throw std::runtime_error("Unknown fixture class");
}

#endif
