#ifndef FIXTUREFUNCTION_H
#define FIXTUREFUNCTION_H

#include <string>
#include <vector>

#include "controlvalue.h"
#include "dmxchannel.h"
#include "namedobject.h"
#include "valuesnapshot.h"

/**
	@author Andre Offringa
*/
class FixtureFunction : public NamedObject {
	public:
		enum FunctionType {
			Brightness,
			RedIntensity, 
			GreenIntensity, 
			BlueIntensity, 
			AmberIntensity, 
			WhiteIntensity,
			UVIntensity,
			Strobe, 
			Pulse, 
			Rotation, 
			ColorMacro, 
			Pan, 
			Tilt
		};

		FixtureFunction(class Theatre &theatre, FunctionType type, const std::string &name);
		
		FixtureFunction(class Theatre &theatre, FunctionType type);
		
		FixtureFunction(const FixtureFunction& source, class Theatre& theatre);

		FixtureFunction(const FixtureFunction& source) = delete;
		FixtureFunction& operator=(const FixtureFunction& source) = delete;
		
		void Mix(unsigned value, MixStyle mixStyle, unsigned *channels, unsigned universe)
		{
			if(IsSingleChannel() && _firstChannel.Universe() == universe)
			{
				MixStyle combiMixStyle =
					ControlValue::CombineMixStyles(mixStyle, _firstChannel.DefaultMixStyle());

				channels[_firstChannel.Channel()] =
					ControlValue::Mix(channels[_firstChannel.Channel()], value, combiMixStyle);
			}
			else throw std::runtime_error("Can't handle 16-bit values or multiple universes");
		}

		void SetChannel(const DmxChannel &channel);
		bool IsSingleChannel() const { return _additionalChannels.size() == 0; }
		//DmxChannel &FirstChannel() { return _firstChannel; }
		const DmxChannel &FirstChannel() const { return _firstChannel; }
		void IncChannel();
		void DecChannel();
		enum FunctionType Type() const
		{
			return _type;
		}
		unsigned char GetValue(const ValueSnapshot &snapshot) const
		{
			return snapshot.GetValue(_firstChannel);
		}
	private:
		class Theatre &_theatre;
		enum FunctionType _type;
		DmxChannel _firstChannel;
		std::vector<DmxChannel> _additionalChannels;
};

#endif
