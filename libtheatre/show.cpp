#include "show.h"

void Show::Mix(unsigned *channelValues, unsigned universe, const Timing& timing)
{
	for(std::vector<Scene*>::iterator i=_runningScenes.begin();i!=_runningScenes.end();++i)
	{
		if((*i)->HasEnd(timing.TimeInMS()))
		{
			std::vector<Scene*>::iterator j = i;
			--i;
			_runningScenes.erase(j);
		} else {
			(*i)->Mix(channelValues, universe, timing);
		}
	}
}

void Show::Clear()
{
	_runningScenes.clear();
	_scenes.clear();
}



