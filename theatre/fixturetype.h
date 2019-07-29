#ifndef FIXTURETYPE_H
#define FIXTURETYPE_H

#include <string>
#include <vector>

#include "color.h"
#include "folderobject.h"
#include "valuesnapshot.h"

/**
	@author Andre Offringa
*/
class FixtureType : public FolderObject {
	public:
		enum FixtureClass {
			Light1Ch,
			RGBLight3Ch,
			RGBLight4Ch,
			RGBALight4Ch,
			RGBALight5Ch,
			RGBWLight4Ch,
			RGBUVLight4Ch,
			UVLight3Ch,
			H2ODMXPro,
			RGB_ADJ_6CH,
			RGB_ADJ_7CH,
			BT_VINTAGE_5CH,
			BT_VINTAGE_6CH,
			BT_VINTAGE_7CH
		};

		FixtureType(FixtureClass fixtureClass) : FolderObject(ClassName(fixtureClass)), _class(fixtureClass)
		{ }
		
		FixtureType(const FixtureType& fixtureType) :
			FolderObject(fixtureType),
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
				case RGBUVLight4Ch:
					return "RGBUV light (4ch)";
				case UVLight3Ch:
					return "UV light (3ch)";
				case H2ODMXPro:
					return "H2O DMX Pro";
				case RGB_ADJ_6CH:
					return "RGB ADJ (6ch)";
				case RGB_ADJ_7CH:
					return "RGB ADJ (7ch)";
				case BT_VINTAGE_5CH:
					return "Briteq Vintage (5ch)";
				case BT_VINTAGE_6CH:
					return "Briteq Vintage (6ch)";
				case BT_VINTAGE_7CH:
					return "Briteq Vintage (7ch)";
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
				RGBUVLight4Ch,
				UVLight3Ch,
				H2ODMXPro,
				RGB_ADJ_6CH,
				RGB_ADJ_7CH,
				BT_VINTAGE_5CH,
				BT_VINTAGE_6CH,
				BT_VINTAGE_7CH
			};
			return list;
		}
		
		Color GetColor(const class Fixture &fixture, const ValueSnapshot &snapshot, size_t shapeIndex) const;

		enum FixtureClass FixtureClass() const { return _class; }
		
		size_t ShapeCount() const {
			switch(_class)
			{
				case Light1Ch:
				case RGBLight3Ch:
				case RGBLight4Ch:
				case RGBALight4Ch:
				case RGBALight5Ch:
				case RGBWLight4Ch:
				case RGBUVLight4Ch:
				case UVLight3Ch:
				case H2ODMXPro:
				case RGB_ADJ_6CH:
				case RGB_ADJ_7CH:
					return 1;
				case BT_VINTAGE_5CH:
				case BT_VINTAGE_6CH:
				case BT_VINTAGE_7CH:
					return 2;
			}
			return 0;
		}
		
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

inline Color FixtureType::GetColor(const Fixture &fixture, const ValueSnapshot &snapshot, size_t shapeIndex) const
{
	switch(_class)
	{
		case Light1Ch:
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
		case RGBUVLight4Ch:
		{
			unsigned char uv = fixture.Functions()[3]->GetValue(snapshot);
			return Color(
				(fixture.Functions()[0]->GetValue(snapshot) + uv/3)/2,
				(fixture.Functions()[1]->GetValue(snapshot))/2,
				(fixture.Functions()[2]->GetValue(snapshot) + uv)/2
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
		case BT_VINTAGE_5CH:
		case BT_VINTAGE_6CH:
		{
			if(shapeIndex == 0)
				return Color(255, 171, 85) * fixture.Functions()[0]->GetValue(snapshot);
			else {
				unsigned char master = fixture.Functions()[1]->GetValue(snapshot);
				size_t strobe = _class == BT_VINTAGE_5CH ? 0 : 1;
				return master * Color(
					fixture.Functions()[2+strobe]->GetValue(snapshot),
					fixture.Functions()[3+strobe]->GetValue(snapshot),
					fixture.Functions()[4+strobe]->GetValue(snapshot)
				);
			}
		}
		case BT_VINTAGE_7CH:
		{
			if(shapeIndex == 0)
				return Color(255, 171, 85) * fixture.Functions()[0]->GetValue(snapshot);
			else {
				unsigned char master = fixture.Functions()[1]->GetValue(snapshot);
				unsigned char colorEffect = fixture.Functions()[6]->GetValue(snapshot);
				if(colorEffect < 8)
					return master * Color(
						fixture.Functions()[3]->GetValue(snapshot),
						fixture.Functions()[4]->GetValue(snapshot),
						fixture.Functions()[5]->GetValue(snapshot)
					);
				else
					return master * Color::BTMacroColor(fixture.Functions()[6]->GetValue(snapshot));
			}
		}
	}
	throw std::runtime_error("Unknown fixture class");
}

#endif
