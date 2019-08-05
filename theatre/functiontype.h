#ifndef FUNCTION_TYPE_H
#define FUNCTION_TYPE_H

#include <string>

enum class FunctionType
{
	Master,
	Red, 
	Green, 
	Blue, 
	White,
	Amber, 
	UV,
	ColorMacro, 
	Strobe, 
	Pulse, 
	Rotation, 
	Pan, 
	Tilt,
	Effect
};

inline char AbbreviatedFunctionType(FunctionType functionType)
{
	switch(functionType)
	{
		case FunctionType::Master: return 'M';
		case FunctionType::Red: return 'R';
		case FunctionType::Green: return 'G';
		case FunctionType::Blue: return 'B';
		case FunctionType::White: return 'W';
		case FunctionType::Amber: return 'A';
		case FunctionType::UV: return 'U';
		case FunctionType::ColorMacro: return 'C';
		case FunctionType::Pulse: return 'P';
		case FunctionType::Strobe: return 'S';
		case FunctionType::Rotation: return 'O';
		case FunctionType::Pan: return '>';
		case FunctionType::Tilt: return '/';
		case FunctionType::Effect : return 'E';
	}
	return 0;
}

inline std::string FunctionTypeDescription(FunctionType functionType)
{
	switch(functionType)
	{
		case FunctionType::Master: return "Master";
		case FunctionType::Red: return "Red";
		case FunctionType::Green: return "Green";
		case FunctionType::Blue: return "Blue";
		case FunctionType::White: return "White";
		case FunctionType::Amber: return "Amber";
		case FunctionType::UV: return "UV";
		case FunctionType::ColorMacro: return "Color";
		case FunctionType::Pulse: return "Pulse";
		case FunctionType::Strobe: return "Strobe";
		case FunctionType::Rotation: return "Rotation";
		case FunctionType::Pan: return "Pan";
		case FunctionType::Tilt: return "Tilt";
		case FunctionType::Effect: return "Effect";
	}
	return "?";
}

#endif
