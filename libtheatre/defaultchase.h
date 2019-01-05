#ifndef DEFAULT_CHASE_H
#define DEFAULT_CHASE_H

#include <vector>

class DefaultChase
{
public:
	enum RunType { IncreasingRun, DecreasingRun, BackAndForthRun, InwardRun, OutwardRun, RandomRun };
	static class Sequence& MakeRunningLight(class Management& management, const std::vector<class Fixture*>& fixtures, const std::vector<class Color>& colors, RunType runType);
	
	static class Sequence& MakeColorVariation(class Management& management, const std::vector<class Fixture*>& fixtures, const std::vector<class Color>& colors, double variation);
	
private:
	static void addColorPresets(class Management& management, class Fixture& f, class PresetCollection& pc, unsigned red, unsigned green, unsigned blue, unsigned master);
};

#endif

