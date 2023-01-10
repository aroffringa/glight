#include "show.h"

#include "folder.h"
#include "management.h"

namespace glight::theatre {

Scene &Show::AddScene(bool in_folder) {
  std::unique_ptr<Scene> &result =
      _scenes.emplace_back(std::make_unique<Scene>(_management));
  if (in_folder) {
    result->SetName(_management.RootFolder().GetAvailableName("scene"));
    _management.RootFolder().Add(*result);
  }
  return *result.get();
}

void Show::Mix(unsigned *channelValues, unsigned universe,
               const Timing &timing) {
  for (std::vector<Scene *>::iterator i = _runningScenes.begin();
       i != _runningScenes.end(); ++i) {
    if ((*i)->HasEnd(timing.TimeInMS())) {
      std::vector<Scene *>::iterator j = i;
      --i;
      _runningScenes.erase(j);
    } else {
      (*i)->Mix(channelValues, universe, timing);
    }
  }
}

void Show::Clear() {
  _runningScenes.clear();
  _scenes.clear();
}

}  // namespace glight::theatre
