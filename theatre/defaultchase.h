#ifndef DEFAULT_CHASE_H
#define DEFAULT_CHASE_H

#include <vector>

class DefaultChase
{
public:
	enum RunType { IncreasingRun, DecreasingRun, BackAndForthRun, InwardRun, OutwardRun, RandomRun };
	
	enum ShiftType { IncreasingShift, DecreasingShift, BackAndForthShift, RandomShift };
	
	enum VUMeterDirection { VUIncreasing, VUDecreasing, VUInward, VUOutward };
	
	static class PresetCollection& MakeColorPreset(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors);
	
	static class Chase& MakeRunningLight(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, RunType runType);
	
	static class Chase& MakeColorVariation(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, double variation);
	
	static class Chase& MakeColorShift(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, ShiftType shiftType);
	
	static class Controllable& MakeVUMeter(class Management& management, class Folder& destination, const std::vector<class Controllable*>& controllables, const std::vector<class Color>& colors, VUMeterDirection direction);
	
private:
	static void addColorPresets(class Management& management, class Controllable& controllable, class PresetCollection& pc, unsigned red, unsigned green, unsigned blue, unsigned master);
};

#endif

