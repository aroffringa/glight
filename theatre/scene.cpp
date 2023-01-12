#include "scene.h"

#include "management.h"

namespace glight::theatre {

Scene::Scene(Management &management)
    : _management(management),
      _mutex(management.Mutex()),
      _items(),
      _nextStartedItem(_items.begin()),
      _currentOffset(0.0),
      _startOffset(0.0),
      _decoder(),
      _audioPlayer(),
      _hasAudio(false),
      _isPlaying(false) {
  initPlayer();
}

Scene::~Scene() { Stop(); }

void Scene::OnSyncUpdate(double offsetInMS) {
  std::lock_guard<std::mutex> lock(_mutex);
  double currentTime = _management.GetOffsetTimeInMS();
  // bias = currentTime - StartTimeInMS() - offsetInMS;

  // We only adjust 5%, to avoid large steps
  _startTimeInMS =
      StartTimeInMS() + (currentTime - StartTimeInMS() - offsetInMS) * 0.05;
}

}  // namespace glight::theatre
