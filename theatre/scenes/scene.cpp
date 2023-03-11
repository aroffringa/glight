#include "scene.h"

#include "../management.h"

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
  double currentTime = _management.GetOffsetTimeInMS();

  // The bias is given by currentTime - StartTimeInMS() - offsetInMS;
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

ControlSceneItem *Scene::AddControlSceneItem(double offset_in_ms,
                                             Controllable &controllable,
                                             size_t input) {
  std::unique_ptr<ControlSceneItem> item =
      std::make_unique<ControlSceneItem>(controllable, input);
  item->SetOffsetInMS(offset_in_ms);
  ControlSceneItem *result = item.get();
  _items.emplace(offset_in_ms, std::move(item));
  resetCurrentOffset();
  std::pair<const Controllable *, size_t> value(&controllable, input);
  if (std::find(controllables_.begin(), controllables_.end(), value) ==
      controllables_.end()) {
    controllables_.emplace_back(value);
  }
  return result;
}

KeySceneItem *Scene::AddKeySceneItem(double offset_in_ms) {
  std::unique_ptr<KeySceneItem> item = std::make_unique<KeySceneItem>();
  item->SetOffsetInMS(offset_in_ms);
  KeySceneItem *result = item.get();
  _items.emplace(offset_in_ms, std::move(item));
  resetCurrentOffset();
  return result;
}

BlackoutSceneItem &Scene::AddBlackoutItem(double offset_in_ms) {
  std::unique_ptr<BlackoutSceneItem> item =
      std::make_unique<BlackoutSceneItem>();
  item->SetOffsetInMS(offset_in_ms);
  BlackoutSceneItem *result = item.get();
  _items.emplace(offset_in_ms, std::move(item));
  resetCurrentOffset();
  return *result;
}

void Scene::ChangeSceneItemStartTime(SceneItem *item, double newOffsetInMS) {
  ItemMap::node_type node = _items.extract(find(item));
  node.mapped()->SetOffsetInMS(newOffsetInMS);
  node.key() = newOffsetInMS;
  _items.insert(std::move(node));
  resetCurrentOffset();
}

void Scene::Remove(SceneItem *item) {
  std::multimap<double, std::unique_ptr<SceneItem>>::iterator iter = find(item);
  // Make sure the object is deleted only at the end of this function:
  std::unique_ptr<SceneItem> owned_item = std::move(iter->second);
  _items.erase(iter);
  if (ControlSceneItem *c_item = dynamic_cast<ControlSceneItem *>(item);
      c_item) {
    RecalculateControllables();
  }
}

void Scene::Start(double timeInMS) {
  _startTimeInMS = timeInMS;
  resetCurrentOffset();
  Stop();
  _startTimeInMS = _startTimeInMS - _startOffset;
  _isPlaying = true;
  _endOfItems = 0;
  if (_hasAudio) _audioPlayer->Play();
}

void Scene::Stop() {
  RestoreFromBlackout(1.0);
  if (_isPlaying) {
    _audioPlayer.reset();
    _decoder.reset();
    _isPlaying = false;
    _startOffset = 0.0;
    initPlayer();
  } else {
    if (_audioPlayer == nullptr) initPlayer();
  }
}

void Scene::skipTo(double offsetInMS) {
  if (_currentOffset > offsetInMS) resetCurrentOffset();
  // "Start" all items that have started since the last tick
  while (_nextStartedItem != _items.end() &&
         _nextStartedItem->first <= offsetInMS) {
    glight::theatre::SceneItem *item =
        _startedItems.emplace_back(_nextStartedItem->second.get());
    item->Start(*this);
    ++_nextStartedItem;
  }
  // "End" all items which duration have passed.
  for (std::vector<SceneItem *>::iterator i = _startedItems.begin();
       i != _startedItems.end(); ++i) {
    if ((*i)->OffsetInMS() + (*i)->DurationInMS() < offsetInMS) {
      --i;
      std::vector<SceneItem *>::iterator removePointer = i;
      ++removePointer;
      _startedItems.erase(removePointer);
    }
  }
  if (ItemsHaveEnd()) ++_endOfItems;
  _currentOffset = offsetInMS;
}

void Scene::RecalculateControllables() {
  std::set<std::pair<const Controllable *, size_t>> controllables;
  for (const std::pair<const double, std::unique_ptr<SceneItem>> &item :
       _items) {
    if (ControlSceneItem *c_item =
            dynamic_cast<ControlSceneItem *>(item.second.get());
        c_item) {
      controllables.emplace(
          std::make_pair(&c_item->GetControllable(), c_item->GetInput()));
    }
  }
  controllables_.assign(controllables.begin(), controllables.end());
}

void Scene::BlackOut(double fade_speed) {
  _storedSourceValues = _management.StoreSourceValues(true);
  _management.BlackOut(true, fade_speed);
}

void Scene::RestoreFromBlackout(double fade_speed) {
  if (!_storedSourceValues.Empty()) {
    _management.LoadSourceValues(_storedSourceValues, true, fade_speed);
    _storedSourceValues.Clear();
  }
}

}  // namespace glight::theatre
