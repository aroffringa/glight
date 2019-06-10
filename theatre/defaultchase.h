#ifndef DEFAULT_CHASE_H
#define DEFAULT_CHASE_H

#include <vector>

class DefaultChase
{
public:
	enum RunType { IncreasingRun, DecreasingRun, BackAndForthRun, InwardRun, OutwardRun, RandomRun };
	
	enum VUMeterDirection { VUIncreasing, VUDecreasing, VUInward, VUOutward };
	
	static class Sequence& MakeRunningLight(class Management& management, class Folder& destination, const std::vector<class Fixture*>& fixtures, const std::vector<class Color>& colors, RunType runType);
	
	static class Chase& MakeColorVariation(class Management& management, class Folder& destination, const std::vector<class Fixture*>& fixtures, const std::vector<class Color>& colors, double variation);
	
	static class Controllable& MakeVUMeter(class Management& management, class Folder& destination, const std::vector<class Fixture*>& fixtures, const std::vector<class Color>& colors, VUMeterDirection direction);
	
private:
	static void addColorPresets(class Management& management, class Fixture& f, class PresetCollection& pc, unsigned red, unsigned green, unsigned blue, unsigned master);
};

#endif

