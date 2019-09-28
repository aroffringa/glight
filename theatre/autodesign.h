#ifndef AUTO_DESIGN_H
#define AUTO_DESIGN_H

#include <vector>

class AutoDesign
{
public:
	enum RunType { IncreasingRun, DecreasingRun, BackAndForthRun, InwardRun, OutwardRun, RandomRun };
	
	enum ShiftType { IncreasingShift, DecreasingShift, BackAndForthShift, RandomShift };
	
	enum VUMeterDirection { VUIncreasing, VUDecreasing, VUInward, VUOutward };
	
	enum IncreasingType { IncForward, IncBackward, IncForwardReturn, IncBackwardReturn };
	
	struct ColorDeduction
	{
		bool whiteFromRGB, amberFromRGB, uvFromRGB;
	};
	
	static class PresetCollection& MakeColorPreset(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, const ColorDeduction& deduction);
	
	static class Chase& MakeRunningLight(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, const ColorDeduction& deduction, RunType runType);
	
	static class Chase& MakeColorVariation(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, const ColorDeduction& deduction, double variation);
	
	static class Chase& MakeColorShift(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, const ColorDeduction& deduction, ShiftType shiftType);
	
	static class Controllable& MakeVUMeter(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, const ColorDeduction& deduction, VUMeterDirection direction);
	
	static class Chase& MakeIncreasingChase(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, const ColorDeduction& deduction, IncreasingType incType);
	
private:
	static void addColorPresets(class Management& management, class Controllable& controllable, class PresetCollection& pc, const Color& color, const ColorDeduction& deduction);
};

#endif

