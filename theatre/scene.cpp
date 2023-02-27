#include "scene.h"

#include "management.h"

#include <iostream>

namespace glight::theatre {

Scene::Scene(Management &management)
    : _management(management),
      _mutex(management.Mutex()),

      _nextStartedItem(_items.begin()),
      _currentOffset(0.0),
      _startOffset(0.0),

      _hasAudio(false),
      _isPlaying(false),
      _endOfItems(0) {
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

void Scene::initPlayer() {
  if (!_audioFilename.empty()) {
    try {
      _audioPlayer.reset();
      _decoder.reset();
      _decoder = std::make_unique<system::FlacDecoder>(_audioFilename);
      _audioPlayer = std::make_unique<system::AudioPlayer>(*_decoder);
      _audioPlayer->SetSyncListener(*this);
      try {  // Don't report an error when seeking fails
        _audioPlayer->Seek(_startOffset);
        _hasAudio = true;
      } catch (std::exception &e) {
        _hasAudio = false;
      }
    } catch (std::exception &e) {
      std::cout << "Could not open player for filename " << _audioFilename
                << '\n';
      _hasAudio = false;
    }
  }
}

}  // namespace glight::theatre
